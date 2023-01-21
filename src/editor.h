/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef EDITOR_H_
#define EDITOR_H_

#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <locale.h>
#include <byteswap.h>
#include <unistd.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <ansi_esc_seq.h>

#define TRUE 1
#define FALSE 0

#define CHX_MODE_DEFAULT       0
#define CHX_MODE_REPLACE       1
#define CHX_MODE_INSERT        2
#define CHX_MODE_TYPE          3
#define CHX_MODE_REPLACE_ASCII 4
#define CHX_MODE_INSERT_ASCII  5
#define CHX_MODE_TYPE_ASCII    6

#define tenter() system("tput smcup")
#define texit() system("tput rmcup")
#define cls() printf(ANSI_ERASE_SCREEN);
#define cur_set(X, Y) printf(ANSI_CUP(%d,%d), Y + 1, X + 1)

#define TPD 1
#define BPD 1
#define PD (TPD + BPD)

#define CHX_LITTLE_ENDIAN 1
#define CHX_BIG_ENDIAN 0

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]
#define BETWEEN_GE1_L2(X, A, B) (X >= min(A, B) && X < max(A, B))
#define CHX_CONTENT_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_PREVIEW_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + 2 * CINST.group_spacing + CINST.bytes_per_row)
#define CHX_CURSOR_X (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((CINST.cursor.pos % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (CINST.cursor.pos % CINST.bytes_in_group) + CINST.cursor.sbpos + CINST.group_spacing)
#define CHX_CURSOR_Y (int) (CINST.cursor.pos / CINST.bytes_per_row - CINST.scroll_pos + TPD)
#define CHX_GET_X(X) (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((X % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (X % CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_GET_Y(X) (int) (X / CINST.bytes_per_row - CINST.scroll_pos + TPD)

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')

void fvoid();

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);

void chx_config_layout(char _np, char** _pl);
void chx_config_layout_global(char _np, char** _pl);

void chx_add_instance(char* fpath);
void chx_remove_instance(int _n);

struct chx_key chx_get_key();
char chx_get_char();
void chx_get_str();

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
void chx_redraw_line();

void chx_main();

union chx_last_action_ptr {
    void (*execute_void)(void);
    void (*execute_cmmd)(char _np, char** _pl);
};

struct chx_last_action {
    union chx_last_action_ptr action;
    char* params_raw;
    char** params;
    char num_params;
    char type;
};

struct chx_cursor {
    long pos;
    long line;
    char sbpos;
};

struct chx_key {
    char val, type;
} __attribute__ ((__packed__));

struct chx_finfo {
    unsigned char* data;
    char* filename;
    long len, num_rows;
};

struct CHX_INSTANCE {
    unsigned char* style_data;
    int copy_buffer_len;
    char* copy_buffer;
    struct chx_finfo fdata;
    struct chx_cursor cursor;
    struct chx_last_action last_action;
    char bytes_per_row;
    char bytes_in_group;
    char group_spacing;
    char row_num_len;
    char min_row_num_len;
    int height;
    int width;
    int x_offset;
    int y_offset;
    int num_rows;
    long scroll_pos;
    long sel_start;
    long sel_stop;
    char parity;
    char endianness;
    char selected;
    char saved;
    char mode;
    char show_inspector;
    char show_preview;
};

extern struct CHX_INSTANCE* CHX_INSTANCES;
extern int CHX_CUR_MAX_INSTANCE;
extern int CHX_SEL_INSTANCE;

extern void* func_exceptions[];


/* OPTIMISATION SETTINGS (COMMENT TO DISABLE) */
#define CHX_SCROLL_SUPPORT

/* GENERAL SETTINGS */
#define CHX_DEFAULT_ENDIANNESS         CHX_LITTLE_ENDIAN
#define CHX_SHOW_PREVIEW_ON_STARTUP    TRUE // can be overridden if screen is small
#define CHX_SHOW_INSPECTOR_ON_STARTUP  TRUE // can be overridden if screen is small
#define CHX_MAX_NUM_INSTANCES             8 // max number of files open at a time
#define CHX_MAX_NUM_PARAMS                8 // max number of parameters for interpreter commands

/* LAYOUT SETTINGS */
#define CHX_FRAME_COLOUR        ANSI_FG_CYAN
#define CHX_UNSAVED_COLOUR      ANSI_FG_RGB(0,240,240)
#define CHX_ASCII_CUR_FORMAT    ANSI_UNDERLINE
#define CHX_ASCII_SELECT_FORMAT ANSI_REVERSE
#define CHX_SELECT_FORMAT       ANSI_REVERSE

#define CHX_BYTES_PER_ROW   16
#define CHX_BYTES_IN_GROUP   1
#define CHX_GROUP_SPACING    1
#define CHX_MIN_ROW_NUM_LEN  4

/* FEATURES (COMMENT TO DISABLE) */
#define CHX_RESIZE_FILE_ON_BACKSPACE
#define CHX_RESIZE_FILE_ON_INSERTION

#endif // EDITOR_H_
