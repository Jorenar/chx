/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "utils.h"

#include <stdio.h>
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

static const char* const special_ascii[128] = {
    ['\0'] = "'\\0'",
    [0x01] = "SOH",
    [0x02] = "STX",
    [0x03] = "ETX",
    [0x04] = "EOT",
    [0x05] = "ENQ",
    [0x06] = "ACK",
    ['\a'] = "'\\a'",
    ['\b'] = "'\\b'",
    ['\t'] = "'\\t'",
    ['\n'] = "'\\n'",
    ['\v'] = "'\\v'",
    ['\f'] = "'\\f'",
    ['\r'] = "'\\r'",
    [0x0E] = "SO",
    [0x0F] = "SI",
    [0x10] = "DLE",
    [0x11] = "DC1",
    [0x12] = "DC2",
    [0x13] = "DC3",
    [0x14] = "DC4",
    [0x15] = "NAK",
    [0x16] = "SYN",
    [0x17] = "ETB",
    [0x18] = "CAN",
    [0x19] = "EM",
    [0x1A] = "SUB",
    [0x1B] = "ESC",
    [0x1C] = "FS",
    [0x1D] = "GS",
    [0x1E] = "RS",
    [0x1F] = "US",
};

void print_ascii_char(char ch)
{
    if (ch >= 0 && special_ascii[ch]) {
        printf("%s", special_ascii[ch]);
    } else if (IS_PRINTABLE(ch)) {
        printf("'%c'", ch);
    } else {
        printf("\ufffd");
    }
}

void print_utf8(char* bbb)
{
    char ch = bbb[0];
    // If ASCII character ....
    if ((ch & 0x80) == 0) {
        print_ascii_char(ch);
    } else {
        // Process UTF-8
        char b[5] = { ch };
        size_t j;
        for (j = 1; (j < 4) && ((bbb[j] & 0xC0) == 0x80); ++j) {
            b[j] = bbb[j];
        }
        b[j] = 0;
        if (j == 1) {
            printf("\ufffd");
        } else {
            printf("'%s'", b); // Print 1 UTF-8 character.
        }
    }
}
