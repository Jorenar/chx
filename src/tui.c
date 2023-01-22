/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "tui.h"

#include <stdio.h>

#include "editor.h"
#include "utils.h"


int chx_content_end()
{
    return (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + CINST.group_spacing);
}

int chx_preview_end()
{
    return (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + 2 * CINST.group_spacing + CINST.bytes_per_row);
}

int chx_cursor_x()
{
    return (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((CINST.cursor.pos % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (CINST.cursor.pos % CINST.bytes_in_group) + CINST.cursor.sbpos + CINST.group_spacing);
}

int chx_cursor_y()
{
    return (CINST.cursor.pos / CINST.bytes_per_row - CINST.scroll_pos + TPD);
}


void chx_scroll_up(int _n)
{
    printf(ANSI_SU(%d), _n);
    while (_n--) {
        chx_draw_line(CINST.scroll_pos - _n + CINST.num_rows - 1);
    }
    chx_draw_header();
    chx_print_status();
}

void chx_scroll_down(int _n)
{
    printf(ANSI_SD(%d), _n);
    while (_n--) {
        chx_draw_line(CINST.scroll_pos + _n);
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

    if (!CINST.selected) {
        CINST.sel_start = CINST.cursor.pos;
    }
    CINST.sel_stop = CINST.cursor.pos;
    if (CINST.selected) {
        long p = CINST.cursor.pos / CINST.bytes_per_row;
        chx_draw_line(p - 1);
        chx_draw_line(p);
        chx_draw_line(p + 1);
    }

    if (CINST.show_inspector) {
        chx_draw_extra();
    }

    if (CINST.show_preview) {
        chx_draw_sidebar();
    }

    // redraw cursor
    cur_set(chx_cursor_x(), chx_cursor_y());
    fflush(stdout);
}

void chx_draw_line(long line)
{
    // calculate line number
    long line_start = line * CINST.bytes_per_row;
    int cur_y = line - CINST.scroll_pos + TPD;

    // print row number
    cur_set(0, cur_y);
    printf(CHX_FRAME_COLOUR "%0*lX " ANSI_RESET "%-*c", CINST.row_num_len, line_start, CINST.group_spacing, ' ');

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
                printf(CHX_UNSAVED_COLOUR "%02X" ANSI_RESET, CINST.fdata.data[i]);
            } else {
                printf("%02X", CINST.fdata.data[i]);
            }
        } else {
            printf("..");
        }

        if (CINST.selected && i == sel_end - 1) {
            printf(ANSI_RESET);
        }
    }

    printf(ANSI_RESET);

    // restore cursor position
    cur_set(chx_cursor_x(), chx_cursor_y());
}

void chx_print_status()
{
    // print current mode setting
    cur_set(0, CINST.height);
    switch (CINST.mode) {
        case CHX_MODE_NORMAL:
            printf(ANSI_ERASE_LINE "[ NORMAL ]");
            break;
        case CHX_MODE_INSERT:
            printf(ANSI_ERASE_LINE "[ INSERT ]");
            break;
        case CHX_MODE_INSERT_ASCII:
            printf(ANSI_ERASE_LINE "[ ASCII INSERT ]");
            break;
        case CHX_MODE_REPLACE:
            printf(ANSI_ERASE_LINE "[ REPLACE ]");
            break;
        case CHX_MODE_REPLACE_ASCII:
            printf(ANSI_ERASE_LINE "[ ASCII REPLACE ]");
            break;
        case CHX_MODE_REPLACE_SINGLE:
            printf(ANSI_ERASE_LINE "[ REPLACE SINGLE ]");
            break;
        case CHX_MODE_REPLACE_SINGLE_ASCII:
            printf(ANSI_ERASE_LINE "[ ASCII REPLACE SINGLE ]");
            break;
        default:
            printf(ANSI_ERASE_LINE "[ UNKNOWN ]");
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

    int offset = (CINST.show_preview) ? chx_preview_end() : chx_content_end();

    // clear bit of screen
    cur_set(offset - 1, 0);
    for (int i = 0; i < CINST.num_rows + PD; i++) {
        printf(ANSI_ERASE_CUR2EOL ANSI_CHA(%d) ANSI_CUD(1), offset);
    }

    // print inspected data
    cur_set(offset, 0);
    printf(ANSI_BOLD);
    printf(ANSI_ERASE_CUR2EOL "Data Inspector:");
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "binary: "BINARY_PATTERN, BYTE_TO_BINARY(buf[0]));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "int8: %i", INT8_AT(&buf));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "int16: %i", (CINST.endianness) ? INT16_AT(&buf) : __bswap_16 (INT16_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "int32: %i", (CINST.endianness) ? INT32_AT(&buf) : __bswap_32 (INT32_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "int64: %li", (CINST.endianness) ? INT64_AT(&buf) : __bswap_64 (INT64_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "uint8: %u", UINT8_AT(&buf));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "uint16: %u", (CINST.endianness) ? UINT16_AT(&buf) : __bswap_16 (UINT16_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "uint32: %u", (CINST.endianness) ? UINT32_AT(&buf) : __bswap_32 (UINT32_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "uint64: %lu", (CINST.endianness) ? UINT64_AT(&buf) : __bswap_64 (UINT64_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    if (IS_PRINTABLE(buf[0])) {
        printf(ANSI_ERASE_CUR2EOL "ANSI char: %c", buf[0]);
    } else {
        printf(ANSI_ERASE_CUR2EOL "ANSI char: \ufffd");
    }
    printf(ANSI_CHA(%d) ANSI_CUD(1) " ", offset);
    printf(ANSI_ERASE_CUR2EOL "wide char: %lc", (CINST.endianness) ? WCHAR_AT(&buf) : __bswap_16 (WCHAR_AT(&buf)));
    printf(ANSI_CHA(%d) ANSI_CUD(1) ANSI_ERASE_CUR2EOL ANSI_CUD(1) " ", offset);
    if (CINST.endianness) {
        printf(ANSI_ERASE_CUR2EOL "[LITTLE ENDIAN]");
    } else {
        printf(ANSI_ERASE_CUR2EOL "[BIG ENDIAN]");
    }
    printf(ANSI_RESET);
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
    cur_set(chx_cursor_x(), chx_cursor_y());
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

    printf(ANSI_CUP(0,0) "%-*c"CHX_FRAME_COLOUR, CINST.row_num_len + CINST.group_spacing, ' ');
    for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++) {
        printf("%02X%-*c", i * CINST.bytes_in_group, CINST.bytes_in_group * 2 + CINST.group_spacing - 2, ' ');
    }
    printf(ANSI_RESET);
}

void chx_draw_contents()
{
    for (long l = CINST.scroll_pos; l < CINST.scroll_pos + CINST.num_rows; ++l) {
        chx_draw_line(l);
    }
}

void chx_draw_sidebar()
{
    // clear bit of screen if inspector is off
    if (!CINST.show_inspector) {
        cur_set(chx_content_end(), 0);
        for (int i = 0; i < CINST.num_rows + PD; i++) {
            printf(ANSI_ERASE_CUR2EOL ANSI_CHA(%d) ANSI_CUD(1) "", chx_content_end());
        }
    }

    cur_set(chx_content_end(), 0);
    printf("%-*c", CINST.bytes_per_row, ' ');

    long sel_begin = min(CINST.sel_start, CINST.sel_stop);
    long sel_end = max(CINST.sel_start, CINST.sel_stop);

    if (CINST.selected && sel_begin < CINST.scroll_pos * CINST.bytes_per_row) {
        printf(CHX_ASCII_SELECT_FORMAT);
    }

    for (long i = CINST.scroll_pos * CINST.bytes_per_row; i < CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row; i++) {
        if (!(i % CINST.bytes_per_row)) {
            cur_set(chx_content_end(), CHX_GET_Y(i));
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
                printf(ANSI_RESET);
            }
        } else if (i == CINST.cursor.pos) {
            printf(ANSI_RESET);
        }
    }

    printf(ANSI_RESET "%-*c", CINST.group_spacing, ' ');
}
