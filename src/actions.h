/* SPDX-License-Identifier:  MIT
 * Copyright 2023 Jorengarenar
 */

#ifndef ACTIONS_H_
#define ACTIONS_H_

void chx_mode_set_default();
void chx_mode_set_visual();
void chx_mode_set_replace();
void chx_mode_set_insert();
void chx_mode_set_replace2();

void chx_cursor_move_vertical_by(int _n);
void chx_cursor_move_horizontal_by(int _n);
void chx_cursor_move_up_by_5();
void chx_cursor_move_down_by_5();
void chx_cursor_move_right_by_5();
void chx_cursor_move_left_by_5();
void chx_cursor_prev_byte();
void chx_cursor_next_byte();
void chx_cursor_move_up();
void chx_cursor_move_down();
void chx_cursor_move_right();
void chx_cursor_move_left();
void chx_start_selection();
void chx_clear_selection();
void chx_exit_with_message(char* _msg);
void chx_exit();
void chx_resize_file(long _n);
void chx_to_line_start();
void chx_to_line_end();
void chx_to_start();
void chx_to_end();
void chx_count_instances(char _np, char** _pl);
void chx_switch_file(char _np, char** _pl);
void chx_open_instance(char _np, char** _pl);
void chx_close_instance(char _np, char** _pl);
void chx_find_next(char _np, char** _pl);
void chx_page_up();
void chx_page_down();
void chx_toggle_inspector();
void chx_toggle_preview();
void chx_revert();
void chx_save();
void chx_save_as(char _np, char** _pl);
void chx_prompt_save_as();
void chx_copy();
void chx_execute_last_action();
void chx_paste_before();
void chx_paste_after();
void chx_clear_buffer();
void chx_set_inst(char _np, char** _pl);
void chx_next_inst();
void chx_prev_inst();
void chx_config_layout(char _np, char** _pl);
void chx_config_layout_global(char _np, char** _pl);
void chx_print_finfo();
void chx_remove_selected();
void chx_delete_selected();
void chx_save_and_quit();
void chx_quit();
void chx_prompt_command();
void chx_set_hexchar(char _c);
void chx_type_hexchar(char _c);
void chx_insert_hexchar(char _c);
void chx_delete_hexchar();
void chx_backspace_hexchar();
void chx_remove_hexchar();
void chx_erase_hexchar();
void chx_set_ascii(char _c);
void chx_type_ascii(char _c);
void chx_insert_ascii(char _c);
void chx_delete_ascii();
void chx_backspace_ascii();
void chx_remove_ascii();
void chx_erase_ascii();

void foo();

#endif // ACTIONS_H_
