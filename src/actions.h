/* SPDX-License-Identifier:  MIT
 * Copyright 2023 Jorengarenar
 */

#ifndef ACTIONS_H_
#define ACTIONS_H_

void chx_mode_set_default(void);
void chx_mode_set_visual(void);
void chx_mode_set_replace(void);
void chx_mode_set_insert(void);
void chx_mode_set_replace2(void);

void chx_cursor_move_vertical_by(int _n);
void chx_cursor_move_horizontal_by(int _n);
void chx_cursor_move_up_by_5(void);
void chx_cursor_move_down_by_5(void);
void chx_cursor_move_right_by_5(void);
void chx_cursor_move_left_by_5(void);
void chx_cursor_prev_byte(void);
void chx_cursor_next_byte(void);
void chx_cursor_move_up(void);
void chx_cursor_move_down(void);
void chx_cursor_move_right(void);
void chx_cursor_move_left(void);
void chx_start_selection(void);
void chx_clear_selection(void);
void chx_exit_with_message(char* _msg);
void chx_exit(void);
void chx_resize_file(long _n);
void chx_to_line_start(void);
void chx_to_line_end(void);
void chx_to_start(void);
void chx_to_end(void);
void chx_count_instances(char _np, char** _pl);
void chx_switch_file(char _np, char** _pl);
void chx_open_instance(char _np, char** _pl);
void chx_close_instance(char _np, char** _pl);
void chx_find_next(char _np, char** _pl);
void chx_page_up(void);
void chx_page_down(void);
void chx_toggle_inspector(void);
void chx_toggle_preview(void);
void chx_revert(void);
void chx_save(void);
void chx_save_as(char _np, char** _pl);
void chx_prompt_save_as(void);
void chx_copy(void);
void chx_execute_last_action(void);
void chx_paste_before(void);
void chx_paste_after(void);
void chx_clear_buffer(void);
void chx_set_inst(char _np, char** _pl);
void chx_next_inst(void);
void chx_prev_inst(void);
void chx_config_layout(char _np, char** _pl);
void chx_config_layout_global(char _np, char** _pl);
void chx_print_finfo(void);
void chx_remove_selected(void);
void chx_delete_selected(void);
void chx_save_and_quit(void);
void chx_quit();
void chx_prompt_command(void);
void chx_set_hexchar(char _c);
void chx_type_hexchar(char _c);
void chx_insert_hexchar(char _c);
void chx_delete_hexchar(void);
void chx_backspace_hexchar(void);
void chx_remove_hexchar(void);
void chx_erase_hexchar(void);
void chx_set_ascii(char _c);
void chx_type_ascii(char _c);
void chx_insert_ascii(char _c);
void chx_delete_ascii(void);
void chx_backspace_ascii(void);
void chx_remove_ascii(void);
void chx_erase_ascii(void);

void foo(void);

#endif // ACTIONS_H_
