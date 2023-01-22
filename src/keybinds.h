/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef KEYBINDS_H_
#define KEYBINDS_H_

#define CHX_CTRL(C)   (C & 0x1F)
#define CHX_ALT(C)    ((C & 0x00FF) | 0x0300)
#define CHX_SHIFT(C)  ((C & 0x00FF) | 0x0200)
#define CHX_CTRL_M(C) ((C & 0x00FF) | 0x0500)

enum Keys {
    KEY_UP     = 0x0141,
    KEY_DOWN   = 0x0142,
    KEY_RIGHT  = 0x0143,
    KEY_LEFT   = 0x0144,

    KEY_DELETE = 0x0133,
    KEY_INSERT = 0x0132,
    KEY_PG_UP  = 0x0135,
    KEY_PG_DN  = 0x0136,
    KEY_HOME   = 0x0148,
    KEY_END    = 0x0146,

    KEY_ENTER  = 0x000A,
    KEY_TAB    = 0x0009,

    KEY_ESCAPE = 0x0100,
};

extern void (*chx_keybinds_global[])(void);
extern void (*chx_keybinds_mode_command[])(void);
extern void (*chx_keybinds_mode_visual[])(void);

#endif // KEYBINDS_H_
