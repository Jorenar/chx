/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "actions.h"

#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "interpreter.h"
#include "utils.h"
#include "tui.h"

// mode {{{

void chx_mode_set_default()
{
    if (CINST.selected) {
        chx_clear_selection();
    }
    chx_mode_set(CHX_MODE_NORMAL);
}

void chx_mode_set_visual()
{
    chx_mode_set(CHX_MODE_VISUAL);
}

void chx_mode_set_insert()
{
    if (!CINST.foo) {
        chx_mode_set(CHX_MODE_INSERT);
    } else {
        chx_mode_set(CHX_MODE_INSERT_ASCII);
    }
}

void chx_mode_set_replace()
{
    if (!CINST.foo) {
        chx_mode_set(CHX_MODE_REPLACE);
    } else {
        chx_mode_set(CHX_MODE_REPLACE_ASCII);
    }
}

void chx_mode_set_replace2()
{
    if (!CINST.foo) {
        chx_mode_set(CHX_MODE_REPLACE_SINGLE);
    } else {
        chx_mode_set(CHX_MODE_REPLACE_SINGLE_ASCII);
    }
}

// }}}

void chx_cursor_move_vertical_by(int _n)
{
    int new_pos = CINST.cursor.pos + _n * CINST.bytes_per_row;
    CINST.cursor.pos = (new_pos >= CINST.bytes_per_row) ? new_pos : CINST.cursor.pos % CINST.bytes_per_row;
    chx_update_cursor();
}

void chx_cursor_move_horizontal_by(int _n)
{
    CINST.cursor.pos += _n;
    chx_update_cursor();
}

void chx_cursor_move_up_by_5()
{
    chx_cursor_move_vertical_by(-5);
}

void chx_cursor_move_down_by_5()
{
    chx_cursor_move_vertical_by(5);
}

void chx_cursor_move_right_by_5()
{
    chx_cursor_move_horizontal_by(5);
}

void chx_cursor_move_left_by_5()
{
    chx_cursor_move_horizontal_by(-5);
}

void chx_cursor_prev_byte()
{
    CINST.cursor.pos--;
    CINST.cursor.sbpos = 0;
    chx_update_cursor();
}

void chx_cursor_next_byte()
{
    CINST.cursor.pos++;
    CINST.cursor.sbpos = 0;
    chx_update_cursor();
}

void chx_cursor_move_up()
{
    CINST.cursor.pos -= (CINST.cursor.pos >= CINST.bytes_per_row) * CINST.bytes_per_row;
    chx_update_cursor();
}

void chx_cursor_move_down()
{
    CINST.cursor.pos += CINST.bytes_per_row;
    chx_update_cursor();
}

void chx_cursor_move_right()
{
    CINST.cursor.pos += (CINST.cursor.sbpos == 1);
    CINST.cursor.sbpos = !CINST.cursor.sbpos;
    chx_update_cursor();
}

void chx_cursor_move_left()
{
    CINST.cursor.pos -= !CINST.cursor.sbpos;
    CINST.cursor.sbpos = !CINST.cursor.sbpos;
    chx_update_cursor();
}

void chx_start_selection()
{
    CINST.sel_start = CINST.cursor.pos;
    CINST.sel_stop = CINST.cursor.pos;
    CINST.selected = 1;
}

void chx_clear_selection()
{
    CINST.selected = 0;
    chx_draw_contents();
    if (CINST.show_preview) {
        chx_draw_sidebar();
    }
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
}

void chx_cursor_select_up()
{
    if (!CINST.selected) {
        chx_start_selection();
    }
    CINST.cursor.pos -= (CINST.cursor.pos >= CINST.bytes_per_row) * CINST.bytes_per_row;
    CINST.cursor.sbpos = 0;
    CINST.sel_stop = CINST.cursor.pos;
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row + 1);
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    chx_update_cursor();
}

void chx_cursor_select_down()
{
    if (!CINST.selected) {
        chx_start_selection();
    }
    CINST.cursor.pos += CINST.bytes_per_row;
    CINST.cursor.sbpos = 0;
    CINST.sel_stop = CINST.cursor.pos;
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row - 1);
    chx_update_cursor();
}

void chx_cursor_select_right()
{
    if (!CINST.selected) {
        chx_start_selection();
    }
    CINST.cursor.pos++;
    CINST.cursor.sbpos = 0;
    CINST.sel_stop = CINST.cursor.pos;
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row - 1);
    chx_update_cursor();
}

void chx_cursor_select_left()
{
    if (!CINST.selected) {
        chx_start_selection();
    }
    CINST.cursor.pos--;
    CINST.cursor.sbpos = 0;
    CINST.sel_stop = CINST.cursor.pos;
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row + 1);
    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    chx_update_cursor();
}

void chx_exit_with_message(char* _msg)
{
    // re-enable key echoing
    struct termios old = { 0 };
    tcgetattr(0, &old);
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);

    // exit
    printf(ANSI_ERASE_SCREEN);
    cur_set(0, 0);
    system("tput rmcup"); // TODO
    printf(_msg);
    exit(0);
}

void chx_exit()
{
    // re-enable key echoing
    struct termios old = { 0 };
    tcgetattr(0, &old);
    old.c_lflag |= ECHO;
    tcsetattr(0, TCSADRAIN, &old);

    // exit
    printf(ANSI_ERASE_SCREEN);
    cur_set(0, 0);
    system("tput rmcup"); // TODO
    exit(0);
}

void chx_swap_endianness()
{
    CINST.endianness = !CINST.endianness;
    if (CINST.show_inspector) {
        chx_draw_extra();
        cur_set(chx_cursor_x(), chx_cursor_y());
        fflush(stdout);
    }
}

void chx_set_endianness_global(char _np, char** _pl)
{
    if (!_np) {
        return;
    }
    switch (_pl[0][0]) {
        case 'l':
        case 'L':
            for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++) {
                CHX_INSTANCES[i].endianness = 1;
            }
            break;
        case 'b':
        case 'B':
            for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++) {
                CHX_INSTANCES[i].endianness = 0;
            }
            break;
        default:
            return;
    }

    if (CINST.show_inspector) {
        chx_draw_extra();
        cur_set(chx_cursor_x(), chx_cursor_y());
        fflush(stdout);
    }
}

void chx_resize_file(long _n)
{
    CINST.fdata.data = recalloc(CINST.fdata.data, CINST.fdata.len, _n);
    CINST.style_data = recalloc(CINST.style_data, (CINST.fdata.len - 1) / 8 + 1, _n / 8 + 1);
    CINST.fdata.len = _n;
}

void chx_to_line_start()
{
    CINST.cursor.pos -= CINST.cursor.pos % CINST.bytes_per_row;
    CINST.cursor.sbpos = 0;
    chx_update_cursor();
}

void chx_to_line_end()
{
    CINST.cursor.pos -= CINST.cursor.pos % CINST.bytes_per_row - CINST.bytes_per_row + 1;
    CINST.cursor.sbpos = 0;
    chx_update_cursor();
}

void chx_to_start()
{
    CINST.cursor = (struct chx_cursor) { 0 };
    chx_update_cursor();
    chx_draw_all();
}

void chx_to_end()
{
    CINST.cursor.pos = CINST.fdata.len - 1;
    CINST.cursor.sbpos = 0;
    chx_update_cursor();

    if (CINST.cursor.line >= CINST.num_rows) {
        CINST.scroll_pos = CINST.cursor.line - CINST.num_rows / 2;
    }

    chx_draw_all();
}

void chx_count_instances(char _np, char** _pl)
{
    if (!_np) {
        return;
    }
    int len = str_len(_pl[0]);
    char* buf = malloc(len + 1);
    buf[len] = 0;

    // count instances in file
    long count = 0;
    for (long i = 0; i <= CINST.fdata.len - len; i++) {
        for (int n = 0; n < len; n++) {
            buf[n] = CINST.fdata.data[i + n];
        }
        if (cmp_str(buf, _pl[0])) {
            count++;
        }
    }

    free(buf);

    // print number of occurances and wait for key input to continue
    cur_set(0, CINST.height);
    printf(ANSI_ERASE_LINE "found %li occurances of '%s' in file '%s'", count, _pl[0], CINST.fdata.filename);
    fflush(stdout);
    chx_get_char();

    // redraw elements
    chx_draw_all();
}

void chx_switch_file(char _np, char** _pl)
{
    if (!_np) {
        return;
    }

    // load file
    struct chx_finfo hdata = chx_import(_pl[0]);
    if (!hdata.data) {
        // alert user file could not be found and wait for key input to continue
        cur_set(0, CINST.height);
        printf(ANSI_ERASE_LINE "file '%s' not found.", _pl[0]);
        fflush(stdout);
        chx_get_char();
        return;
    }
    hdata.filename = memfork(_pl[0], str_len(_pl[0]) + 1);

    // update instance
    free(CINST.fdata.filename);
    CINST.fdata = hdata;
    CINST.saved = 1;

    // remove highlighting for unsaved data
    free(CINST.style_data);
    CINST.style_data = calloc(1, CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0));

    // redraw elements
    chx_draw_all();
}

void chx_open_instance(char _np, char** _pl)
{
    if (!_np) {
        return;
    }
    chx_add_instance(_pl[0]);
    chx_draw_all();
}

void chx_close_instance(char _np, char** _pl)
{
    int inst;
    if (str_is_num(_pl[0])) {
        inst = str_to_num(_pl[0]);
    } else {
        inst = CHX_SEL_INSTANCE;
    }
    chx_remove_instance(inst);
    chx_draw_all();
}

void chx_find_next(char _np, char** _pl)
{
    if (!_np) {
        return;
    }
    int len = str_len(_pl[0]);
    char* buf = malloc(len + 1);
    buf[len] = 0;

    // look for first occurance starting at cursor pos.
    long b = CINST.cursor.pos + 1;
    for (long i = 0; !cmp_str(buf, _pl[0]) && i < CINST.fdata.len; i++, b++) {
        if (b >= CINST.fdata.len) {
            b = 0;
        }
        for (int n = 0; n < len; n++) {
            buf[n] = CINST.fdata.data[b + n];
        }
    }

    free(buf);

    // update cursor pos to start of occurance
    CINST.cursor.pos = b - 1;
    chx_update_cursor();
}

void chx_page_up()
{
    if (CINST.scroll_pos > 0) {
        CINST.scroll_pos--;
#ifdef CHX_SCROLL_SUPPORT
        chx_scroll_down(1);
#else
        chx_draw_contents();
#endif
        chx_cursor_move_up();
    }
}

void chx_page_down()
{
    CINST.scroll_pos++;
#ifdef CHX_SCROLL_SUPPORT
    chx_scroll_up(1);
#else
    chx_draw_contents();
#endif
    chx_cursor_move_down();
}

void chx_toggle_inspector()
{
    CINST.show_inspector = !CINST.show_inspector;
    printf(ANSI_ERASE_SCREEN);
    chx_draw_all();
}

void chx_toggle_preview()
{
    CINST.show_preview = !CINST.show_preview;
    printf(ANSI_ERASE_SCREEN);
    chx_draw_all();
}


void chx_revert()
{
    // reload file to remove unsaved changes
    char* old_filename = CINST.fdata.filename;
    CINST.fdata = chx_import(old_filename);
    CINST.fdata.filename = old_filename;
    CINST.saved = 1;

    // remove highlighting for unsaved data
    free(CINST.style_data);
    CINST.style_data = calloc(1, CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0));

    // redraw elements
    chx_draw_all();
}

void chx_save()
{
    // remove highlighting for unsaved data
    for (int i = 0; i < CINST.fdata.len / 8; i++) {
        CINST.style_data[i] = 0;
    }

    // export file and redraw file contents
    CINST.saved = 1;
    chx_export(CINST.fdata.filename);
    chx_draw_contents();
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
}

void chx_save_as(char _np, char** _pl)
{
    if (_np) {
        free(CINST.fdata.filename);
        CINST.fdata.filename = memfork(_pl[0], str_len(_pl[0]) + 1);
    }

    // remove highlighting for unsaved data
    for (int i = 0; i < CINST.fdata.len / 8; i++) {
        CINST.style_data[i] = 0;
    }

    // export file and redraw file contents
    CINST.saved = 1;
    chx_export(CINST.fdata.filename);
    chx_draw_contents();
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
}

void chx_prompt_save_as()
{
    chx_draw_all();

    // setup user input buffer
    char usrin[256];

    // print save dialoge and recieve user input
    cur_set(0, CINST.height);
    printf(ANSI_ERASE_LINE "SAVE AS? (LEAVE EMPTY TO CANCEL): ");
    fflush(stdout);

    chx_get_str(usrin, 256);

    // null terminate input at first newline
    char* filename = chx_extract_param(usrin, 0);

    // only export if filename was entered
    if (filename[0]) {
        chx_export(filename);
        CINST.saved = 1;
        for (int i = 0; i < CINST.fdata.len / 8; i++) {
            CINST.style_data[i] = 0;
        }
    }

    // redraw elements
    printf(ANSI_ERASE_SCREEN);
    chx_draw_all();
}

int copy_buffer_len = 0;
char* copy_buffer = NULL;

void chx_copy()
{
    long sel_begin = min(CINST.sel_start, CINST.sel_stop);
    copy_buffer_len = chx_abs(CINST.sel_start - CINST.sel_stop);
    if (copy_buffer_len + sel_begin > CINST.fdata.len) {
        copy_buffer_len -= copy_buffer_len + sel_begin - CINST.fdata.len;
    }
    copy_buffer = realloc(copy_buffer, copy_buffer_len); // TODO
    for (int i = 0; i < copy_buffer_len; i++) {
        copy_buffer[i] = CINST.fdata.data[sel_begin + i];
    }
}

void chx_execute_last_action()
{
    if (CINST.last_action.type) {
        CINST.last_action.action.execute_cmmd(CINST.last_action.num_params, CINST.last_action.params);
    } else {
        CINST.last_action.action.execute_void();
    }
}

void chx_paste_before()
{
    CINST.saved = 0;

    // scroll if pasting past visible screen
    if (CINST.cursor.pos - copy_buffer_len < CINST.scroll_pos * CINST.bytes_per_row) {
        CINST.scroll_pos = ((CINST.cursor.pos - copy_buffer_len) / CINST.bytes_per_row > 0) ? (CINST.cursor.pos - CINST.copy_buffer_len) / CINST.bytes_per_row : 0;
    }

    // resize file if pasting past end
    if (CINST.cursor.pos > CINST.fdata.len) {
        chx_resize_file(CINST.cursor.pos + 1);
    }

    // copy data into file buffer
    for (int i = 0; i < copy_buffer_len && CINST.cursor.pos - i > 0; i++) {
        CINST.fdata.data[CINST.cursor.pos - i] = copy_buffer[CINST.copy_buffer_len - i - 1];
        CINST.style_data[(CINST.cursor.pos - i) / 8] |= 0x80 >> ((CINST.cursor.pos - i) % 8);
    }

    // move cursor to beginning of pasted data
    CINST.cursor.pos -= copy_buffer_len;
    CINST.cursor.sbpos = 1;
    chx_draw_contents();
    chx_update_cursor();
}

void chx_paste_after()
{
    CINST.saved = 0;

    // scroll if pasting past visible screen
    if (CINST.cursor.pos + copy_buffer_len > CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row) {
        CINST.scroll_pos = (CINST.cursor.pos + copy_buffer_len - CINST.num_rows * CINST.bytes_per_row) / CINST.bytes_per_row + 1;
    }

    // resize file if pasting past end
    if (CINST.cursor.pos + copy_buffer_len > CINST.fdata.len) {
        chx_resize_file(CINST.cursor.pos + copy_buffer_len);
    }

    // copy data into file buffer
    for (int i = 0; i < copy_buffer_len; i++) {
        CINST.fdata.data[CINST.cursor.pos + i] = copy_buffer[i];
        CINST.style_data[(CINST.cursor.pos + i) / 8] |= 0x80 >> ((CINST.cursor.pos + i) % 8);
    }

    // move cursor to end of pasted data
    CINST.cursor.pos += copy_buffer_len;
    CINST.cursor.sbpos = 0;
    chx_draw_contents();
    chx_update_cursor();
}

void chx_clear_buffer()
{
    free(copy_buffer);
    copy_buffer = 0;
    copy_buffer_len = 0;
}

void chx_set_inst(char _np, char** _pl)
{
    if (!str_is_num(_pl[0])) {
        return;
    }
    int inst = str_to_num(_pl[0]);
    if (inst <= CHX_CUR_MAX_INSTANCE && inst >= 0) {
        CHX_SEL_INSTANCE = inst;
    }
    chx_draw_all();
}

void chx_next_inst()
{
    if (CHX_SEL_INSTANCE < CHX_CUR_MAX_INSTANCE) {
        CHX_SEL_INSTANCE++;
    } else {
        CHX_SEL_INSTANCE = 0;
    }
    chx_draw_all();
}

void chx_prev_inst()
{
    if (CHX_SEL_INSTANCE > 0) {
        CHX_SEL_INSTANCE--;
    } else {
        CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
    }
    chx_draw_all();
}

void chx_config_layout(char _np, char** _pl)
{
    if (_np < 2) {
        return;
    }

    char* prop_ptr = 0;

    if (cmp_str("rnl", _pl[0])) {
        prop_ptr = &CINST.min_row_num_len;
    } else if (cmp_str("gs", _pl[0])) {
        prop_ptr = &CINST.group_spacing;
    } else if (cmp_str("bpr", _pl[0])) {
        prop_ptr = &CINST.bytes_per_row;
    } else if (cmp_str("big", _pl[0])) {
        prop_ptr = &CINST.bytes_in_group;
    }

    if (prop_ptr && str_is_num(_pl[1])) {
        *prop_ptr = (str_to_num(_pl[1])) ? str_to_num(_pl[1]) : 1;
    }

    printf(ANSI_ERASE_SCREEN);
    chx_draw_all();
}

void chx_config_layout_global(char _np, char** _pl)
{
    for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++) {
        chx_config_layout(_np, _pl);
        chx_next_inst();
    }
}

void chx_print_finfo()
{
    // count lines
    int nlc = 1;
    int chc = 0;
    for (int i = 0; i < CINST.fdata.len; i++) {
        if (IS_PRINTABLE(CINST.fdata.data[i])) {
            chc++;
        } else if (CINST.fdata.data[i] == 0x0A) {
            nlc++;
        }
    }

    // print info and for key input to ocntinue
    cur_set(0, CINST.height);
    printf(ANSI_ERASE_LINE "'%s' %liB %iL %iC (offset: %#lx)", CINST.fdata.filename, CINST.fdata.len, nlc, chc, CINST.cursor.pos);
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
    chx_get_char();

    // redraw elements
    chx_draw_all();
}

void chx_remove_selected()
{
    if (CINST.selected) {
        long sel_begin = min(CINST.sel_start, CINST.sel_stop);
        long sel_end = max(CINST.sel_start, CINST.sel_stop);
        long sel_size = sel_end - sel_begin;
        CINST.saved = 0;
        if (sel_end > CINST.fdata.len - 1) {
            chx_resize_file(sel_begin);
        } else {
            for (int i = sel_end + 1; i < CINST.fdata.len; i++) {
                CINST.fdata.data[i - sel_size] = CINST.fdata.data[i];
            }
            chx_resize_file(CINST.fdata.len - sel_size);
        }
        CINST.cursor.pos = (sel_begin > 0) ? sel_begin : 0;
        CINST.cursor.sbpos = 0;
        chx_clear_selection();
        chx_draw_all();
    }
}

void chx_delete_selected()
{
    if (CINST.selected) {
        long sel_begin = min(CINST.sel_start, CINST.sel_stop);
        long sel_end = max(CINST.sel_start, CINST.sel_stop);
        CINST.saved = 0;
        if (sel_end > CINST.fdata.len - 1) {
            chx_resize_file(sel_begin);
        } else {
            for (int i = sel_begin; i < sel_end; i++) {
                CINST.fdata.data[i] = 0;
                CINST.style_data[i / 8] |= 0x80 >> (i % 8);
            }
        }
        CINST.cursor.pos = (sel_begin > 0) ? sel_begin : 0;
        CINST.cursor.sbpos = 0;
        chx_clear_selection();
        chx_draw_all();
    }
}

void chx_save_and_quit()
{
    chx_export(CINST.fdata.filename);
    chx_exit();
}

void chx_quit()
{
    chx_draw_all();

    // ask user if they would like to save
    if (!CINST.saved) {
        cur_set(0, CINST.height);
        printf(ANSI_ERASE_LINE "WOULD YOU LIKE TO SAVE? (Y / N): ");
        fflush(stdout);

        switch (chx_get_char()) {
            case 'y':
            case 'Y':
                chx_export(CINST.fdata.filename);
                break;
            default:
                // erase save dialoge and redraw elements
                printf(ANSI_ERASE_SCREEN);
                chx_draw_all();
                chx_main();
                break;
            case 'n':
            case 'N':
                break;
        }
    }

    chx_exit();
}

void chx_prompt_command()
{
    // setup user input buffer
    char* usrin = calloc(1, 256);

    // command interpreter recieve user input
    cur_set(0, CINST.height);
    printf(ANSI_ERASE_LINE ":");
    fflush(stdout);

    chx_get_str(usrin, 256);

    // extract command and its parameters
    char np = 0;
    char* cmd = chx_extract_param(usrin, 0);
    char** p = malloc(CHX_MAX_NUM_PARAMS * sizeof (void*));

    for (int i = 0; i < CHX_MAX_NUM_PARAMS; i++) {
        p[i] = chx_extract_param(usrin, i + 1);
        if (p[i][0]) {
            np++;
        }
    }

    // lookup entered command and execute procedure
    // for numbers (decimal or hex, prefixed with '0x') jump to the corresponging byte
    if (cmd[0]) {
        if (str_is_num(cmd)) {
            CINST.cursor.pos = str_to_num(cmd);
            CINST.cursor.sbpos = 0;
            chx_update_cursor();
            chx_draw_all();
        } else if (str_is_hex(cmd)) {
            CINST.cursor.pos = str_to_hex(cmd);
            CINST.cursor.sbpos = 0;
            chx_update_cursor();
            chx_draw_all();
        } else {
            for (int i = 0; chx_void_commands[i].str; i++) {
                if (cmp_str(chx_void_commands[i].str, cmd)) {
                    chx_void_commands[i].execute();
                    CINST.last_action.action.execute_void = chx_void_commands[i].execute;
                    CINST.last_action.type = 0;
                    break;
                }
            }

            for (int i = 0; chx_commands[i].str; i++) {
                if (cmp_str(chx_commands[i].str, cmd)) {
                    chx_commands[i].execute(np, p);
                    free(CINST.last_action.params_raw);
                    free(CINST.last_action.params);
                    CINST.last_action.action.execute_cmmd = chx_commands[i].execute;
                    CINST.last_action.params_raw = usrin;
                    CINST.last_action.num_params = np;
                    CINST.last_action.params = p;
                    CINST.last_action.type = 1;
                    break;
                }
            }
        }
    }

    // redraw elements
    chx_draw_all();
}

void chx_set_hexchar(char _c)
{
    if (!IS_CHAR_HEX(_c)) {
        return;                   // only accept hex characters
    }
    if ((_c ^ 0x60) < 7) {
        _c -= 32;                  // ensure everything is upper-case

    }
    char nullkey[2] = { _c, 0 };

    // resize file if typing past current file length
    if (CINST.cursor.pos >= CINST.fdata.len) {
        chx_resize_file(CINST.cursor.pos + 1);
        chx_draw_contents();
    }

    // update stored file data
    CINST.fdata.data[CINST.cursor.pos] &= 0x0F << (CINST.cursor.sbpos * 4);
    CINST.fdata.data[CINST.cursor.pos] |= strtol(nullkey, NULL, 16) << (!CINST.cursor.sbpos * 4);

    // highlight unsaved change
    CINST.saved = 0;
    CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);

    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    chx_update_cursor();
}

void chx_type_hexchar(char _c)
{
    chx_set_hexchar(_c);
    chx_cursor_move_right();
}

void chx_insert_hexchar_old(char _c)
{
    chx_set_hexchar(_c);
    if (CINST.cursor.sbpos) {
        chx_resize_file(CINST.fdata.len + 1);
        for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos; i--) {
            CINST.fdata.data[i] = CINST.fdata.data[i - 1];
        }
        CINST.fdata.data[CINST.cursor.pos + 1] = 0;
        chx_draw_all();
    }
    chx_cursor_move_right();
}

void chx_insert_hexchar(char _c)
{
#ifdef CHX_RESIZE_FILE_ON_INSERTION
    CINST.parity = !CINST.parity;
    if (CINST.parity && CINST.cursor.pos < CINST.fdata.len) {
        chx_resize_file(CINST.fdata.len + 1);
    }
#endif

    // resize file if typing past current file length
    if (CINST.cursor.pos >= CINST.fdata.len) {
        chx_resize_file(CINST.cursor.pos + 1);
        chx_draw_contents();
    }

    // shift data after cursor by 4 bits
    unsigned char cr = 0;

    for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos - !CINST.cursor.sbpos; i--) {
        cr = CINST.fdata.data[i - 1] & 0x0F;
        CINST.fdata.data[i] >>= 4;
        CINST.fdata.data[i] |= cr << 4;
    }

    // hightlight as unsaved change
    CINST.saved = 0;
    CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);

    // type hexchar and move cursor
    chx_set_hexchar(_c);
    chx_cursor_move_right();

    chx_draw_all();
    fflush(stdout);
}

void chx_delete_hexchar()
{
    // only delete if cursor is before EOF
    if (CINST.cursor.pos < CINST.fdata.len) {
        if (CINST.cursor.sbpos) {
            CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
        } else {
            CINST.fdata.data[CINST.cursor.pos] &= 0x0F;
        }
    }

    // hightlight as unsaved change
    CINST.saved = 0;
    CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);

    if (CINST.show_preview) {
        chx_draw_sidebar();
    }

    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    fflush(stdout);
}

void chx_backspace_hexchar()
{
    chx_cursor_move_left();
    chx_delete_hexchar();
}

void chx_remove_hexchar()
{
    // only remove characters in the file
    if (CINST.cursor.pos < CINST.fdata.len) {
        CINST.saved = 0;

        // shift data after cursor by 4 bits
        if (CINST.cursor.sbpos) {
            CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
        } else {
            CINST.fdata.data[CINST.cursor.pos] <<= 4;
        }

        unsigned char cr = 0;

        for (int i = CINST.cursor.pos; i < CINST.fdata.len - 1; i++) {
            cr = CINST.fdata.data[i + 1] & 0xF0;
            CINST.fdata.data[i] |= cr >> 4;
            CINST.fdata.data[i + 1] <<= 4;
        }

#ifdef CHX_RESIZE_FILE_ON_BACKSPACE
        CINST.parity = !CINST.parity;
        if (!CINST.parity) {
            chx_resize_file(CINST.fdata.len - 1);
        }
#endif

        chx_draw_all();
        fflush(stdout);
    } else if (CINST.cursor.pos == CINST.fdata.len - 1 && CINST.cursor.sbpos) {
        // if cursor is just after EOF, resize file to remove last byte
        chx_resize_file(CINST.fdata.len - 1);
        CINST.cursor.sbpos = 0;
    }
}

void chx_erase_hexchar()
{
    if (CINST.cursor.pos || CINST.cursor.sbpos) {
        chx_cursor_move_left();
        chx_remove_hexchar();
    }
}

void chx_set_ascii(char _c)
{
    // resize file if typing past current file length
    if (CINST.cursor.pos >= CINST.fdata.len) {
        chx_resize_file(CINST.cursor.pos + 1);
        chx_draw_contents();
    }

    // set char
    CINST.fdata.data[CINST.cursor.pos] = _c;

    // highlight unsaved change
    CINST.saved = 0;
    CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);

    if (CINST.show_preview) {
        chx_draw_sidebar();
    }

    chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
    fflush(stdout);
}

void chx_type_ascii(char _c)
{
    chx_set_ascii(_c);
    chx_cursor_next_byte();
}

void chx_insert_ascii(char _c)
{
    // resize file
    chx_resize_file(CINST.fdata.len + 1);

    // shift bytes after cursor right by one
    for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos; i--) {
        CINST.fdata.data[i] = CINST.fdata.data[i - 1];
    }

    // type char
    chx_type_ascii(_c);

    // update screen
    chx_draw_all();
    fflush(stdout);
}

void chx_delete_ascii()
{
    // only delete if cursor is before EOF
    if (CINST.cursor.pos < CINST.fdata.len) {
        chx_set_ascii(0);

        // highlight unsaved change
        CINST.saved = 0;
        CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
    }
}

void chx_backspace_ascii()
{
    chx_cursor_prev_byte();
    chx_delete_ascii();
}

void chx_remove_ascii()
{
    // only remove characters in the file
    if (CINST.cursor.pos < CINST.fdata.len) {
        // shift bytes after cursor left by one
        CINST.cursor.sbpos = 0;
        for (int i = CINST.cursor.pos; i < CINST.fdata.len - 1; i++) {
            CINST.fdata.data[i] = CINST.fdata.data[i + 1];
        }

        // resize file
        chx_resize_file(CINST.fdata.len - 1);

        // redraw contents and update cursor
        chx_draw_all();
        fflush(stdout);
    }
}

void chx_erase_ascii()
{
    if (CINST.cursor.pos) {
        CINST.cursor.sbpos = 0;
        chx_cursor_prev_byte();
        chx_remove_ascii();
    }
}

void foo()
{
    CINST.foo = !CINST.foo;
    chx_update_cursor();
}
