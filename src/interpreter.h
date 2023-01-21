/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

struct chx_void_command {
    void (*execute)(void);
    char* str;
};

struct chx_command {
    void (*execute)(char _np, char** _pl);
    char* str;
};

extern struct chx_void_command chx_void_commands[];
extern struct chx_command chx_commands[];

#endif // INTERPRETER_H_
