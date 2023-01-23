/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "keybinds.h"

#include "actions.h"
#include "interpreter.h"

#define KEYBINDS_ARRAY_SIZE  0xFFFF + 1

/* GLOBAL KEYBINDS (WORK IN ANY MODE) */
void (*chx_keybinds_global[KEYBINDS_ARRAY_SIZE])(void) = {
    [KEY_ESCAPE] = chx_mode_set_default,
    [KEY_UP] = chx_cursor_move_up,
    [KEY_DOWN] = chx_cursor_move_down,
    [KEY_RIGHT] = chx_cursor_next_byte,
    [KEY_LEFT] = chx_cursor_prev_byte,
    [CHX_ALT(KEY_UP)] = chx_cursor_move_up_by_5,
    [CHX_ALT(KEY_DOWN)] = chx_cursor_move_down_by_5,
    [CHX_ALT(KEY_RIGHT)] = chx_cursor_move_right_by_5,
    [CHX_ALT(KEY_LEFT)] = chx_cursor_move_left_by_5,
    [CHX_CTRL_M(KEY_UP)] = chx_cursor_move_up,
    [CHX_CTRL_M(KEY_DOWN)] = chx_cursor_move_down,
    [CHX_CTRL_M(KEY_RIGHT)] = chx_cursor_move_right,
    [CHX_CTRL_M(KEY_LEFT)] = chx_cursor_move_left,
    [CHX_CTRL('y')] = chx_copy,
    [CHX_CTRL('p')] = chx_paste_after,
    [CHX_SHIFT(CHX_CTRL('p'))] = chx_paste_before,
    [CHX_CTRL('e')] = chx_swap_endianness,
    [CHX_CTRL('w')] = chx_prompt_save_as,
    [CHX_CTRL('u')] = chx_revert,
    [CHX_CTRL('x')] = chx_exit,
    [KEY_DELETE] = chx_delete_hexchar,
    [KEY_PG_UP] = chx_page_up,
    [KEY_PG_DN] = chx_page_down,
    [CHX_CTRL_M(KEY_PG_UP)] = chx_next_inst,
    [CHX_CTRL_M(KEY_PG_DN)] = chx_prev_inst,
};

/* COMMAND MODE KEYBINDS */
void (*chx_keybinds_mode_command[KEYBINDS_ARRAY_SIZE])(void) = {
    ['h'] = chx_cursor_move_left,
    ['j'] = chx_cursor_move_down,
    ['l'] = chx_cursor_move_right,
    ['k'] = chx_cursor_move_up,
    ['b'] = chx_cursor_prev_byte,
    ['w'] = chx_cursor_next_byte,
    [CHX_ALT('k')] = chx_cursor_move_up_by_5,
    [CHX_ALT('j')] = chx_cursor_move_down_by_5,
    [CHX_ALT('l')] = chx_cursor_move_right_by_5,
    [CHX_ALT('h')] = chx_cursor_move_left_by_5,
    ['u'] = chx_revert,
    ['y'] = chx_copy,
    ['P'] = chx_paste_before,
    ['p'] = chx_paste_after,
    ['X'] = chx_erase_ascii,
    ['x'] = chx_remove_ascii,
    ['G'] = chx_to_end,
    ['g'] = chx_to_start,
    ['v'] = chx_mode_set_visual,
    ['R'] = chx_mode_set_replace,
    ['i'] = chx_mode_set_insert,
    ['r'] = chx_mode_set_replace2,
    ['q'] = chx_quit,
    ['.'] = chx_execute_last_action,
    ['^'] = chx_to_line_start,
    ['0'] = chx_to_line_start,
    ['$'] = chx_to_line_end,
    [':'] = chx_prompt_command,
    [KEY_TAB] = foo,
};

void (*chx_keybinds_mode_visual[KEYBINDS_ARRAY_SIZE])(void) = {
    ['h'] = chx_cursor_move_left,
    ['j'] = chx_cursor_move_down,
    ['l'] = chx_cursor_move_right,
    ['k'] = chx_cursor_move_up,
    ['D'] = chx_remove_selected,
    ['d'] = chx_delete_selected,
};
