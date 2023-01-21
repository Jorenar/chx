/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "editor.h"

#include "interpreter.h"
#include "actions.h"
#include "keybinds.h"
#include "utils.h"

struct CHX_INSTANCE* CHX_INSTANCES;
int CHX_CUR_MAX_INSTANCE;
int CHX_SEL_INSTANCE;

/* FUNCTIONS TO EXCLUDE FROM ACTION HISTORY */
void* func_exceptions[] = {
    chx_cursor_move_up,
    chx_cursor_move_down,
    chx_cursor_move_right,
    chx_cursor_move_left,
    chx_cursor_select_up,
    chx_cursor_select_down,
    chx_cursor_select_right,
    chx_cursor_select_left,
    chx_cursor_next_byte,
    chx_cursor_prev_byte,
    chx_execute_last_action,
    chx_prompt_command,
    chx_mode_set_type_ascii,
    chx_mode_set_type,
    chx_mode_set_insert_ascii,
    chx_mode_set_insert,
    chx_mode_set_replace_ascii,
    chx_mode_set_replace,
    chx_remove_selected,
    chx_delete_selected,
    chx_erase_hexchar,
    chx_remove_hexchar,
    chx_erase_ascii,
    chx_remove_ascii,
    chx_open_instance,
    chx_close_instance,
    chx_set_inst,
    chx_switch_file,
};

void fvoid() {};

struct chx_finfo chx_import(char* fpath)
{
    struct chx_finfo finfo;
    FILE* inf = fopen(fpath, "r+b");
    if (!inf) {
        return (struct chx_finfo) { 0 };
    }
    fseek(inf, 0, SEEK_END);
    long flen = ftell(inf);
    rewind(inf);
    finfo.len = flen;
    finfo.data = malloc(flen);
    fread(finfo.data, 1, flen, inf);
    fclose(inf);
    return finfo;
}

void chx_export(char* fpath)
{
    FILE* outf = fopen(fpath, "w+b");
    fwrite(CINST.fdata.data, 1, CINST.fdata.len, outf);
    fclose(outf);
}

void chx_scroll_up(int _n)
{
    printf("\e[%dS", _n);
    while (_n--) {
        chx_redraw_line(CINST.scroll_pos - _n + CINST.num_rows - 1);
    }
    chx_draw_header();
    chx_print_status();
}

void chx_scroll_down(int _n)
{
    printf("\e[%dT", _n);
    while (_n--) {
        chx_redraw_line(CINST.scroll_pos + _n);
    }
    chx_draw_header();
    chx_print_status();
}

void chx_update_cursor()
{
    CINST.cursor.line = CINST.cursor.pos / CINST.bytes_per_row;
    if (CINST.cursor.pos < 0) {
        CINST.cursor = (struct chx_cursor) { 0 };
    } else {
        // scroll if past visible screen
        int scroll_pos_prev = CINST.scroll_pos;
        if (CINST.cursor.line >= CINST.scroll_pos + CINST.num_rows) {
            CINST.scroll_pos = CINST.cursor.line - CINST.num_rows + 1;
#ifdef CHX_SCROLL_SUPPORT
            if (CINST.scroll_pos < scroll_pos_prev + CINST.num_rows) {
                chx_scroll_up(CINST.scroll_pos - scroll_pos_prev);
            } else {
                chx_draw_contents();
            }
#else
            chx_draw_contents();
#endif
        } else if (CINST.cursor.line < CINST.scroll_pos) {
            CINST.scroll_pos = CINST.cursor.line;
#ifdef CHX_SCROLL_SUPPORT
            if (scroll_pos_prev < CINST.scroll_pos + CINST.num_rows) {
                chx_scroll_down(scroll_pos_prev - CINST.scroll_pos);
            } else {
                chx_draw_contents();
            }
#else
            chx_draw_contents();
#endif
        }
    }

    if (CINST.show_inspector) {
        chx_draw_extra();
    }

    if (CINST.show_preview) {
        chx_draw_sidebar();
    }

    // redraw cursor
    cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
    fflush(stdout);
}

void chx_redraw_line(long line)
{
    // calculate line number
    long line_start = line * CINST.bytes_per_row;
    int cur_y = line - CINST.scroll_pos + TPD;

    // print row number
    cur_set(0, cur_y);
    printf(CHX_FRAME_COLOUR "%0*lX \e[0m%-*c", CINST.row_num_len, line_start, CINST.group_spacing, ' ');

    // print row contents
    long sel_begin = min(CINST.sel_start, CINST.sel_stop);
    long sel_end = max(CINST.sel_start, CINST.sel_stop);

    cur_set(CINST.row_num_len + CINST.group_spacing, cur_y);

    if (CINST.selected && line_start >= sel_begin && line_start < sel_end) {
        printf(CHX_ASCII_SELECT_FORMAT);
    }

    for (long i = line_start; i < line_start + CINST.bytes_per_row; i++) {
        if (i != line_start && !(i % CINST.bytes_in_group) && CINST.group_spacing != 0) {
            printf("%-*c", CINST.group_spacing, ' ');
        }

        if (CINST.selected && i == sel_begin && sel_end != sel_begin) {
            printf(CHX_ASCII_SELECT_FORMAT);
        }

        if (i < CINST.fdata.len) {
            if ((!CINST.selected || i < sel_begin || i >= sel_end) && CINST.style_data[i / 8] & (0x80 >> (i % 8))) {
                printf(CHX_UNSAVED_COLOUR "%02X\e[0m", CINST.fdata.data[i]);
            } else {
                printf("%02X", CINST.fdata.data[i]);
            }
        } else {
            printf("..");
        }

        if (CINST.selected && i == sel_end - 1) {
            printf("\e[0m");
        }
    }

    printf("\e[0m");

    // restore cursor position
    cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
}

void chx_print_status()
{
    // print current mode setting
    cur_set(0, CINST.height);
    switch (CINST.mode) {
        case CHX_MODE_DEFAULT:
            printf("\e[2K[ COMMAND ]");
            break;
        case CHX_MODE_TYPE:
            printf("\e[2K[ TYPE ]");
            break;
        case CHX_MODE_INSERT:
            printf("\e[2K[ INSERT ]");
            break;
        case CHX_MODE_REPLACE:
            printf("\e[2K[ REPLACE ]");
            break;
        case CHX_MODE_TYPE_ASCII:
            printf("\e[2K[ ASCII TYPE ]");
            break;
        case CHX_MODE_INSERT_ASCII:
            printf("\e[2K[ ASCII INSERT ]");
            break;
        case CHX_MODE_REPLACE_ASCII:
            printf("\e[2K[ ASCII REPLACE ]");
            break;
        default:
            printf("\e[2K[ UNKNOWN ]");
            break;
    }
    printf(" I%02i '%s' (%li bytes)", CHX_SEL_INSTANCE, CINST.fdata.filename, CINST.fdata.len);
}

void chx_draw_extra()
{
    // copy bytes from file
    char buf[16];
    for (long i = 0; i < 16; i++) {
        if (CINST.cursor.pos + i < CINST.fdata.len) {
            buf[i] = CINST.fdata.data[CINST.cursor.pos + i];
        } else {
            buf[i] = 0;
        }
    }

    int offset = (CINST.show_preview) ? CHX_PREVIEW_END : CHX_CONTENT_END;

    // clear bit of screen
    cur_set(offset - 1, 0);
    for (int i = 0; i < CINST.num_rows + PD; i++) {
        printf("\e[0K\e[%dG\e[1B", offset);
    }

    // print inspected data
    cur_set(offset, 0);
    printf("\e[1m");
    printf("\e[0KData Inspector:");
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kbinary: "BINARY_PATTERN, BYTE_TO_BINARY(buf[0]));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kint8: %i", INT8_AT(&buf));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kint16: %i", (CINST.endianness) ? INT16_AT(&buf) : __bswap_16 (INT16_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kint32: %i", (CINST.endianness) ? INT32_AT(&buf) : __bswap_32 (INT32_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kint64: %li", (CINST.endianness) ? INT64_AT(&buf) : __bswap_64 (INT64_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kuint8: %u", UINT8_AT(&buf));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kuint16: %u", (CINST.endianness) ? UINT16_AT(&buf) : __bswap_16 (UINT16_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kuint32: %u", (CINST.endianness) ? UINT32_AT(&buf) : __bswap_32 (UINT32_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kuint64: %lu", (CINST.endianness) ? UINT64_AT(&buf) : __bswap_64 (UINT64_AT(&buf)));
    printf("\e[%dG\e[1B ", offset);
    if (IS_PRINTABLE(buf[0])) {
        printf("\e[0KANSI char: %c", buf[0]);
    } else {
        printf("\e[0KANSI char: \ufffd");
    }
    printf("\e[%dG\e[1B ", offset);
    printf("\e[0Kwide char: %lc", (CINST.endianness) ? WCHAR_AT(&buf) : __bswap_16 (WCHAR_AT(&buf)));
    printf("\e[%dG\e[1B\e[0K\e[1B ", offset);
    if (CINST.endianness) {
        printf("\e[0K[LITTLE ENDIAN]");
    } else {
        printf("\e[0K[BIG ENDIAN]");
    }
    printf("\e[0m");
}

void chx_draw_all()
{
    // draw elements
    chx_draw_header();
    chx_draw_contents();

    if (CINST.show_preview) {
        chx_draw_sidebar();
    }

    if (CINST.show_inspector) {
        chx_draw_extra();
    }

    chx_print_status();

    // restore cursor position
    cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
    fflush(stdout);
}

void chx_draw_header()
{
    int rnum_digits = chx_count_digits((CINST.scroll_pos + CINST.num_rows) * CINST.bytes_per_row - 1);
    int rnl_old = CINST.row_num_len;

    if (rnum_digits > CINST.min_row_num_len) {
        CINST.row_num_len = rnum_digits;
    } else {
        CINST.row_num_len = CINST.min_row_num_len;
    }

    if (rnl_old != CINST.row_num_len) {
        chx_draw_all();
    }

    printf("\e[0;0H%-*c"CHX_FRAME_COLOUR, CINST.row_num_len + CINST.group_spacing, ' ');
    for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++) {
        printf("%02X%-*c", i * CINST.bytes_in_group, CINST.bytes_in_group * 2 + CINST.group_spacing - 2, ' ');
    }
    printf("\e[0m");
}

void chx_draw_contents()
{
    // print row numbers
    for (int i = 0; i < CINST.num_rows; i++) {
        cur_set(0, i + TPD);
        printf(CHX_FRAME_COLOUR "%0*lX \e[0m%-*c", CINST.row_num_len, (long) ((i + CINST.scroll_pos) * CINST.bytes_per_row), CINST.group_spacing, ' ');
    }

    printf("\e[0m");

    // print main contents
    long sel_begin = min(CINST.sel_start, CINST.sel_stop);
    long sel_end = max(CINST.sel_start, CINST.sel_stop);

    if (CINST.selected && sel_begin < CINST.scroll_pos * CINST.bytes_per_row) {
        printf(CHX_ASCII_SELECT_FORMAT);
    }

    for (long i = CINST.scroll_pos * CINST.bytes_per_row; i < CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row; i++) {
        if (!(i % CINST.bytes_per_row)) {
            printf("%-*c", CINST.group_spacing, ' ');
            cur_set(CINST.row_num_len + CINST.group_spacing, CHX_GET_Y(i));
        } else if (!(i % CINST.bytes_in_group) && CINST.group_spacing != 0) {
            printf("%-*c", CINST.group_spacing, ' ');
        }

        if (CINST.selected && i == sel_begin && sel_end != sel_begin) {
            printf(CHX_ASCII_SELECT_FORMAT);
        }

        if (i < CINST.fdata.len) {
            if ((!CINST.selected || i < sel_begin || i >= sel_end) && CINST.style_data[i / 8] & (0x80 >> (i % 8))) {
                printf(CHX_UNSAVED_COLOUR "%02X\e[0m", CINST.fdata.data[i]);
            } else {
                printf("%02X", CINST.fdata.data[i]);
            }
        } else {
            printf("..");
        }

        if (CINST.selected && i == sel_end - 1) {
            printf("\e[0m");
        }
    }

    printf("\e[0m%-*c", CINST.group_spacing, ' ');
}

void chx_draw_sidebar()
{
    // clear bit of screen if inspector is off
    if (!CINST.show_inspector) {
        cur_set(CHX_CONTENT_END, 0);
        for (int i = 0; i < CINST.num_rows + PD; i++) {
            printf("\e[0K\e[%dG\e[1B", CHX_CONTENT_END);
        }
    }

    cur_set(CHX_CONTENT_END, 0);
    printf("%-*c", CINST.bytes_per_row, ' ');

    long sel_begin = min(CINST.sel_start, CINST.sel_stop);
    long sel_end = max(CINST.sel_start, CINST.sel_stop);

    if (CINST.selected && sel_begin < CINST.scroll_pos * CINST.bytes_per_row) {
        printf(CHX_ASCII_SELECT_FORMAT);
    }

    for (long i = CINST.scroll_pos * CINST.bytes_per_row; i < CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row; i++) {
        if (!(i % CINST.bytes_per_row)) {
            cur_set(CHX_CONTENT_END, CHX_GET_Y(i));
        }

        if (i == CINST.cursor.pos && !CINST.selected) {
            printf(CHX_ASCII_CUR_FORMAT);
        }

        if (i < CINST.fdata.len) {
            if (CINST.selected && i == sel_begin && sel_end != sel_begin) {
                printf(CHX_ASCII_SELECT_FORMAT);
            }

            if (IS_PRINTABLE(CINST.fdata.data[i])) {
                printf("%c", CINST.fdata.data[i]);
            } else {
                printf("·");
            }
        } else {
            printf("•");
        }

        if (CINST.selected) {
            if (i == sel_end - 1) {
                printf("\e[0m");
            }
        } else if (i == CINST.cursor.pos) {
            printf("\e[0m");
        }
    }

    printf("\e[0m%-*c", CINST.group_spacing, ' ');
}

struct chx_key chx_get_key()
{
    struct chx_key key;
    char buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // set terminal flags and enter raw mode
    struct termios old = { 0 };
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);

    read(0, &buf, 8);

    // check the type of key press (ALT, ESCAPE, etc.)
    switch (buf[0]) {
        case 0x1B:
            if (buf[5] == '~') {
                key = (struct chx_key) { buf[2], buf[4] - 0x30 };
            } else if (buf[5]) {
                key = (struct chx_key) { buf[5], buf[4] - 0x30 };
            } else if (buf[1] == 0x5B || !buf[1]) {
                key = (struct chx_key) { buf[2], 0x01 };
            } else {
                key = (struct chx_key) { buf[1], 0x03 };
            }
            break;
        default:
            key = (struct chx_key) { buf[0], 0 };
            break;
    }

    // restore flags and re-enter cooked mode
    old.c_lflag |= ICANON;
    tcsetattr(0, TCSADRAIN, &old);

    return key;
}

char chx_get_char()
{
    char buf;

    // set terminal flags and enter raw mode
    struct termios old = { 0 };
    tcgetattr(0, &old);
    old.c_lflag &= ~ICANON;
    old.c_lflag |= ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &old);

    read(0, &buf, 1);

    // restore flags and re-enter cooked mode
    old.c_lflag |= ICANON;
    old.c_lflag &= ~ECHO;
    tcsetattr(0, TCSADRAIN, &old);

    return buf;
}

void chx_get_str(char* _buf, int _len)
{
    struct chx_key k;
    for (int bpos = 0; WORD(k) != 0x0A; k = chx_get_key()) {
        if (WORD(k) == KEY_LEFT && bpos > 0) {
            bpos--;
            printf("\e[1D");
        } else if (WORD(k) == KEY_RIGHT && _buf[bpos]) {
            bpos++;
            printf("\e[1C");
        } else if (WORD(k) == 0x7F && bpos) {
            bpos--;
            int n;
            for (n = bpos; _buf[n]; n++) {
                _buf[n] = _buf[n + 1];
            }
            printf("\e[1D");
            for (n = bpos; _buf[n]; n++) {
                printf("%c", _buf[n]);
            }
            printf("\e[0K");
            if (n - bpos) {
                printf("\e[%iD", n - bpos);
            }

        } else if (IS_PRINTABLE(WORD(k))) {
            int n = 0;
            while (_buf[n]) {
                n++;
            }
            for (n++; n > 0; n--) {
                _buf[bpos + n] = _buf[bpos + n - 1];
            }
            _buf[bpos] = k.val;
            for (n = bpos; _buf[n]; n++) {
                printf("%c", _buf[n]);
            }
            printf("\e[0K");
            printf("\e[%iD", n - bpos);
            bpos++;
            printf("\e[1C");
        } else if (WORD(k) == KEY_ESCAPE) {
            for (int i = 0; _buf[i]; i++) {
                _buf[i] = 0;
            }
            return;
        }
        fflush(stdout);
    }
}

void chx_add_instance(char* fpath)
{
    if (CHX_CUR_MAX_INSTANCE >= CHX_MAX_NUM_INSTANCES - 1) {
        return;
    }

    // load file
    struct chx_finfo hdata = chx_import(fpath);
    if (!hdata.data) {
        // alert user file could not be found and wait for key input to continue
        cur_set(0, CINST.height);
        printf("\e[2Kfile '%s' not found.", fpath);
        fflush(stdout);
        chx_get_char();
        return;
    }

    hdata.filename = memfork(fpath, str_len(fpath) + 1);

    // get window dimensions
    struct winsize size;
    ioctl(0, TIOCGWINSZ, (char*) &size);

    // setup instance
    CHX_CUR_MAX_INSTANCE++;
    CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
    CINST.cursor = (struct chx_cursor) { 0 };
    CINST.fdata = hdata;
    CINST.style_data = calloc(1, hdata.len / 8 + (hdata.len % 8 != 0));
    CINST.height = size.ws_row;
    CINST.width = size.ws_col;
    CINST.x_offset = 0;
    CINST.y_offset = 0;
    CINST.bytes_per_row = CHX_BYTES_PER_ROW;
    CINST.bytes_in_group = CHX_BYTES_IN_GROUP;
    CINST.group_spacing = CHX_GROUP_SPACING;
    CINST.min_row_num_len = CINST.row_num_len = CHX_MIN_ROW_NUM_LEN;
    CINST.num_rows = size.ws_row - PD;
    CINST.endianness = CHX_DEFAULT_ENDIANNESS;
    CINST.last_action.action.execute_void = fvoid;
    CINST.last_action.params = NULL;
    CINST.last_action.params_raw = NULL;
    CINST.last_action.type = 0;
    CINST.copy_buffer = NULL;
    CINST.saved = 1;

    CINST.show_inspector = (CHX_PREVIEW_END + 28 > CINST.width) ? 0 : CHX_SHOW_INSPECTOR_ON_STARTUP;
    CINST.show_preview = (CHX_PREVIEW_END > CINST.width) ? 0 : CHX_SHOW_PREVIEW_ON_STARTUP;
}

void chx_remove_instance(int _n)
{
    if (_n < 0 || _n > CHX_CUR_MAX_INSTANCE || !CHX_CUR_MAX_INSTANCE) {
        return;
    }

    CHX_CUR_MAX_INSTANCE--;
    if (CHX_SEL_INSTANCE > CHX_CUR_MAX_INSTANCE) {
        CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
    }

    // empty struct
    free(CHX_INSTANCES[_n].copy_buffer);
    free(CHX_INSTANCES[_n].style_data);
    free(CHX_INSTANCES[_n].fdata.data);
    free(CHX_INSTANCES[_n].fdata.filename);

    CHX_INSTANCES[_n] = (struct CHX_INSTANCE) { 0 };

    // shift instances to remove spaces
    for (int i = 0; i < CHX_MAX_NUM_INSTANCES - 1; i++) {
        if (!CHX_INSTANCES[i].style_data) {
            CHX_INSTANCES[i] = CHX_INSTANCES[i + 1];
        }
    }
}

void chx_main()
{
    for (struct chx_key key;; key = chx_get_key()) {
        // execute key sequence, if available
        if (chx_keybinds_global[WORD(key)]) {
            chx_keybinds_global[WORD(key)]();

            // make sure function is not in exclusion list, if so then set the last action to the function pointer
            char is_valid = 1;
            for (int i = 0; i < sizeof (func_exceptions) / sizeof (void*); i++) {
                if (chx_keybinds_global[WORD(key)] == func_exceptions[i]) {
                    is_valid = 0;
                }
            }
            if (is_valid) {
                CINST.last_action.action.execute_void = chx_keybinds_global[WORD(key)];
                CINST.last_action.type = 0;
            }
        }

        switch (CINST.mode) {
            default:
            case CHX_MODE_DEFAULT:
                // execute key sequence, if available
                if (chx_keybinds_mode_command[WORD(key)]) {
                    chx_keybinds_mode_command[WORD(key)]();

                    // make sure function is not in exclusion list, if so then set the last action to the function pointer
                    char is_valid = 1;
                    for (int i = 0; i < sizeof (func_exceptions) / sizeof (void*); i++) {
                        if (chx_keybinds_mode_command[WORD(key)] == func_exceptions[i]) {
                            is_valid = 0;
                        }
                    }
                    if (is_valid) {
                        CINST.last_action.action.execute_void = chx_keybinds_mode_command[WORD(key)];
                        CINST.last_action.type = 0;
                    }
                }
                break;
            case CHX_MODE_INSERT:
                if (IS_CHAR_HEX(WORD(key))) {
                    chx_insert_hexchar(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_erase_hexchar();
                }
                break;
            case CHX_MODE_REPLACE:
                if (IS_CHAR_HEX(WORD(key))) {
                    chx_set_hexchar(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_delete_hexchar();
                }
                break;
            case CHX_MODE_TYPE:
                if (IS_CHAR_HEX(WORD(key))) {
                    chx_type_hexchar(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_backspace_hexchar();
                }
                break;
            case CHX_MODE_INSERT_ASCII:
                if (IS_PRINTABLE(WORD(key))) {
                    chx_insert_ascii(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_erase_ascii();
                }
                break;
            case CHX_MODE_REPLACE_ASCII:
                if (IS_PRINTABLE(WORD(key))) {
                    chx_set_ascii(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_delete_ascii();
                }
                break;
            case CHX_MODE_TYPE_ASCII:
                if (IS_PRINTABLE(WORD(key))) {
                    chx_type_ascii(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_backspace_ascii();
                }
                break;
        }

        // clear selection if cursor is not at the end of the selection (meaning the user is no longer selecting and the cursor has moved)
        if (CINST.selected && CINST.cursor.pos != CINST.sel_stop) {
            chx_clear_selection();
        }
    }
}
