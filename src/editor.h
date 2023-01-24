/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdbool.h>

#include <byteswap.h>
#include <unistd.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <ansi_esc_seq.h>

enum Mode {
    CHX_MODE_NORMAL,

    CHX_MODE_VISUAL,

    CHX_MODE_INSERT,
    CHX_MODE_INSERT_ASCII,

    CHX_MODE_REPLACE,
    CHX_MODE_REPLACE_ASCII,

    CHX_MODE_REPLACE_SINGLE,
    CHX_MODE_REPLACE_SINGLE_ASCII,
};

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);

void chx_config_layout(char _np, char** _pl);
void chx_config_layout_global(char _np, char** _pl);

void chx_add_instance(char* fpath);
void chx_remove_instance(int _n);

struct chx_key chx_get_key();
char chx_get_char();
void chx_get_str();

void chx_mode_set(enum Mode m);

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
    char sbpos; // ??
};

struct chx_key {
    char val, type;
} __attribute__ ((__packed__));

struct chx_finfo {
    unsigned char* data;
    char* filename;
    long len, num_rows;
};

struct Instance {
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
    char selected;
    char saved;
    char mode;
    char show_inspector;
    char show_preview;
    bool foo;
};

extern struct Instance* CHX_INSTANCES;
extern int CHX_CUR_MAX_INSTANCE;
extern int CHX_SEL_INSTANCE;

extern void* func_exceptions[];


// TODO move to config

/* GENERAL SETTINGS */
#define CHX_SHOW_PREVIEW_ON_STARTUP    1 // (bool) can be overridden if screen is small
#define CHX_SHOW_INSPECTOR_ON_STARTUP  1 // (bool) can be overridden if screen is small
#define CHX_MAX_NUM_INSTANCES          8 // max number of files open at a time
#define CHX_MAX_NUM_PARAMS             8 // max number of parameters for interpreter commands

/* LAYOUT SETTINGS */
#define CHX_FRAME_COLOUR        ANSI_FG_GRAY
#define CHX_UNSAVED_COLOUR      ANSI_FG_RGB(0,240,240)
#define CHX_ASCII_CUR_FORMAT    ANSI_REVERSE
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
