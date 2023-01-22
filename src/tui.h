/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef TUI_H_
#define TUI_H_

#define TPD 1 // top line padding
#define BPD 1 // bottom line padding
#define PD (TPD + BPD)

#define CHX_SCROLL_SUPPORT // TODO programically

int chx_content_end();
int chx_preview_end();
int chx_cursor_x();
int chx_cursor_y();

// TODO check if needed
#define CHX_GET_X(X) (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((X % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (X % CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_GET_Y(X) (int) (X / CINST.bytes_per_row - CINST.scroll_pos + TPD)

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"

void chx_scroll_up(int _n);
void chx_scroll_down(int _n);

void chx_print_status();
void chx_update_cursor();
void chx_swap_endianness();
void chx_draw_contents();
void chx_draw_sidebar();
void chx_draw_extra();
void chx_draw_header();
void chx_draw_all();
void chx_draw_line();

#endif // TUI_H_
