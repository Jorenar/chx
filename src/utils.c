/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "utils.h"

#include <stdlib.h>

long chx_abs(long _n)
{
    return _n + 2 * _n * -(_n < 0);
}

long min(long _a, long _b)
{
    if (_a < _b) {
        return _a;
    }
    return _b;
}

long max(long _a, long _b)
{
    if (_a > _b) {
        return _a;
    }
    return _b;
}

char* memfork(char* _p, int _l)
{
    char* np = malloc(_l);
    for (int i = 0; i < _l; i++) {
        np[i] = _p[i];
    }
    return np;
}

char* recalloc(char* _p, long _o, long _n)
{
    char* ptr = calloc(1, _n);
    for (int i = 0; i < min(_o, _n); i++) {
        ptr[i] = _p[i];
    }
    free(_p);
    return ptr;
}

int chx_count_digits(long _n)
{
    int c = 0;
    while ((_n /= 16) >= 1) {
        c++;
    }
    return ++c;
}

char str_is_num(char* _s)
{
    if (!_s[0]) {
        return 0;
    }
    for (int i = 0; _s[i]; i++) {
        if (!IS_DIGIT(_s[i])) {
            return 0;
        }
    }
    return 1;
}

char str_is_hex(char* _s)
{
    if (!(_s[0] == '0' && _s[1] == 'x')) {
        return 0;
    }
    for (int i = 2; _s[i]; i++) {
        if (!IS_CHAR_HEX(_s[i])) {
            return 0;
        }
    }
    return 1;
}

int str_to_num(char* _s)
{
    long total = 0;
    for (int i = 0; _s[i]; i++) {
        total = total * 10 + _s[i] - 0x30;
    }
    return total;
}

long str_to_hex(char* _s)
{
    long total = 0;
    for (int i = 2; _s[i]; i++) {
        total *= 16;
        if ((_s[i] ^ 0x60) < 7) {
            _s[i] -= 32;
        }
        total += (_s[i] > 0x40) ? _s[i] - 0x37 : _s[i] - 0x30;
    }
    return total;
}

int str_len(char* _s)
{
    int c = 0;
    while (_s[c]) {
        c++;
    }
    return c;
}

char cmp_str(char* _a, char* _b)
{
    for (int i = 0; _a[i] || _b[i]; i++) {
        if (_a[i] != _b[i]) {
            return 0;
        }
    }
    return 1;
}

char* chx_extract_param(char* _s, int _n)
{
    int n;
    // extract param
    char* param = _s;
    for (int i = 0; i < _n; i++) {
        for (n = 0; param[n] > 0x20 && param[n] < 0x7F; n++);
        param += n + 1;
    }

    // terminate param at first non-typable char or space (' ', '\n', '\t', etc.)
    char qf = 0;
    for (n = 0; param[n] > 0x20 - qf && param[n] < 0x7F; n++) {
        if (IS_QUOTATION(param[n])) {
            qf = !qf;
        }
    }

    if (IS_QUOTATION(param[0]) && IS_QUOTATION(param[n - 1])) {
        *(param++ + n - 1) = 0;
    } else {
        param[n] = 0;
    }

    return param;
}
