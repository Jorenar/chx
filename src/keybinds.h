/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#include "editor.h"


#define CHX_CTRL(C)   (C & 0x1F)
#define CHX_ALT(C)    ((C & 0x00FF) | 0x0300)
#define CHX_SHIFT(C)  ((C & 0x00FF) | 0x0200)
#define CHX_CTRL_M(C) ((C & 0x00FF) | 0x0500)


#define KEY_UP      0x0141
#define KEY_DOWN    0x0142
#define KEY_RIGHT   0x0143
#define KEY_LEFT    0x0144

#define KEY_DELETE  0x0133
#define KEY_INSERT  0x0132
#define KEY_PG_UP   0x0135
#define KEY_PG_DN   0x0136
#define KEY_HOME    0x0148
#define KEY_END     0x0146

#define KEY_ENTER   0x000A
#define KEY_TAB     0x0009

#define KEY_ESCAPE  0x0100
#define KEY_MAX_VAL 0xFFFF


extern void (*chx_keybinds_global[])(void);
extern void (*chx_keybinds_mode_command[])(void);
extern void (*chx_keybinds_mode_visual[])(void);

#endif // KEYBINDS_H_
