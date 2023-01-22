/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "editor.h"

#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"
#include "actions.h"
#include "keybinds.h"
#include "tui.h"
#include "utils.h"

struct Instance* CHX_INSTANCES;
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
    chx_mode_set_replace_ascii,
    chx_mode_set_replace,
    chx_mode_set_insert_ascii,
    chx_mode_set_insert,
    chx_mode_set_replace2_ascii,
    chx_mode_set_replace2,
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
            printf(ANSI_CUB(1));
        } else if (WORD(k) == KEY_RIGHT && _buf[bpos]) {
            bpos++;
            printf(ANSI_CUF(1));
        } else if (WORD(k) == 0x7F && bpos) {
            bpos--;
            int n;
            for (n = bpos; _buf[n]; n++) {
                _buf[n] = _buf[n + 1];
            }
            printf(ANSI_CUB(1));
            for (n = bpos; _buf[n]; n++) {
                printf("%c", _buf[n]);
            }
            printf(ANSI_ERASE_CUR2EOL "");
            if (n - bpos) {
                printf(ANSI_CUB(%i), n - bpos);
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
            printf(ANSI_ERASE_CUR2EOL "");
            printf(ANSI_CUB(%i), n - bpos);
            bpos++;
            printf(ANSI_CUF(1));
        } else if (WORD(k) == KEY_ESCAPE) {
            for (int i = 0; _buf[i]; i++) {
                _buf[i] = 0;
            }
            return;
        }
        fflush(stdout);
    }
}

void chx_mode_set(enum Mode m)
{
    CINST.mode = m;
    chx_print_status();
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
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
        printf(ANSI_ERASE_LINE "file '%s' not found.", fpath);
        fflush(stdout);
        chx_get_char();
        return;
    }

    hdata.filename = memfork(fpath, str_len(fpath) + 1);

    // get window dimensions // TODO: check if shouldn't be global to editor
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

    CINST.show_inspector = (chx_preview_end() + 28 > CINST.width) ? 0 : CHX_SHOW_INSPECTOR_ON_STARTUP;
    CINST.show_preview = (chx_preview_end() > CINST.width) ? 0 : CHX_SHOW_PREVIEW_ON_STARTUP;
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

    CHX_INSTANCES[_n] = (struct Instance) { 0 };

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
            case CHX_MODE_VISUAL:
                if (chx_keybinds_mode_visual[WORD(key)]) {
                    chx_keybinds_mode_visual[WORD(key)]();

                    // make sure function is not in exclusion list, if so then set the last action to the function pointer
                    char is_valid = 1;
                    for (int i = 0; i < sizeof (func_exceptions) / sizeof (void*); i++) {
                        if (chx_keybinds_mode_visual[WORD(key)] == func_exceptions[i]) {
                            is_valid = 0;
                        }
                    }
                    if (is_valid) {
                        CINST.last_action.action.execute_void = chx_keybinds_mode_visual[WORD(key)];
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
            case CHX_MODE_REPLACE2:
                if (IS_CHAR_HEX(WORD(key))) {
                    chx_set_hexchar(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_delete_hexchar();
                }
                break;
            case CHX_MODE_REPLACE:
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
            case CHX_MODE_REPLACE2_ASCII:
                if (IS_PRINTABLE(WORD(key))) {
                    chx_set_ascii(WORD(key));
                } else if (WORD(key) == 0x7F) {
                    chx_delete_ascii();
                }
                break;
            case CHX_MODE_REPLACE_ASCII:
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
