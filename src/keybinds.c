/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "keybinds.h"

#include "actions.h"
#include "interpreter.h"

/* GLOBAL KEYBINDS (WORK IN ANY MODE) */
void (*chx_keybinds_global[])(void) = {
    [KEY_MAX_VAL] = fvoid, // do not remove
    [KEY_ESCAPE] = chx_mode_set_default,
    [KEY_UP] = chx_cursor_move_up,
    [KEY_DOWN] = chx_cursor_move_down,
    [KEY_RIGHT] = chx_cursor_next_byte,
    [KEY_LEFT] = chx_cursor_prev_byte,
    [CHX_SHIFT(KEY_UP)] = chx_cursor_select_up,
    [CHX_SHIFT(KEY_DOWN)] = chx_cursor_select_down,
    [CHX_SHIFT(KEY_RIGHT)] = chx_cursor_select_right,
    [CHX_SHIFT(KEY_LEFT)] = chx_cursor_select_left,
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
    [CHX_CTRL('r')] = chx_replace_mode_toggle,
    [CHX_CTRL('i')] = chx_insert_mode_toggle,
    [CHX_CTRL('t')] = chx_replace2_mode_toggle,
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
void (*chx_keybinds_mode_command[])(void) = {
    [KEY_MAX_VAL] = fvoid, // do not remove
    ['k'] = chx_cursor_move_up,
    ['j'] = chx_cursor_move_down,
    ['l'] = chx_cursor_next_byte,
    ['h'] = chx_cursor_prev_byte,
    ['K'] = chx_cursor_select_up,
    ['J'] = chx_cursor_select_down,
    ['L'] = chx_cursor_select_right,
    ['H'] = chx_cursor_select_left,
    [CHX_ALT('k')] = chx_cursor_move_up_by_5,
    [CHX_ALT('j')] = chx_cursor_move_down_by_5,
    [CHX_ALT('l')] = chx_cursor_move_right_by_5,
    [CHX_ALT('h')] = chx_cursor_move_left_by_5,
    [CHX_CTRL('k')] = chx_cursor_move_up,
    [CHX_CTRL('j')] = chx_cursor_move_down,
    [CHX_CTRL('l')] = chx_cursor_move_right,
    [CHX_CTRL('h')] = chx_cursor_move_left,
    ['u'] = chx_revert,
    ['y'] = chx_copy,
    ['P'] = chx_paste_before,
    ['p'] = chx_paste_after,
    ['V'] = chx_erase_hexchar,
    ['v'] = chx_remove_hexchar,
    ['X'] = chx_erase_ascii,
    ['x'] = chx_remove_ascii,
    ['G'] = chx_to_end,
    ['g'] = chx_to_start,
    ['D'] = chx_remove_selected,
    ['d'] = chx_delete_selected,
    ['R'] = chx_mode_set_replace_ascii,
    ['r'] = chx_mode_set_replace,
    ['I'] = chx_mode_set_insert_ascii,
    ['i'] = chx_mode_set_insert,
    ['T'] = chx_mode_set_replace2_ascii,
    ['t'] = chx_mode_set_replace2,
    ['q'] = chx_quit,
    ['.'] = chx_execute_last_action,
    ['^'] = chx_to_line_start,
    ['$'] = chx_to_line_end,
    [':'] = chx_prompt_command,
};
