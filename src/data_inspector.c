/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "data_inspector.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <byteswap.h> // TODO: find platform independent solution?

#include <ansi_esc_seq.h>

#include "tui.h"
#include "editor.h"
#include "utils.h"

// place cursor at the right offset
#define pco(n) printf(ANSI_CHA(%d) ANSI_CUD(1) " " ANSI_ERASE_CUR2EOL, n)

#define INT8_AT(X)   *((int8_t*) (X))
#define INT16_AT(X)  *((int16_t*) (X))
#define INT32_AT(X)  *((int32_t*) (X))
#define INT64_AT(X)  *((int64_t*) (X))
#define UINT8_AT(X)  *((uint8_t*) (X))
#define UINT16_AT(X) *((uint16_t*) (X))
#define UINT32_AT(X) *((uint32_t*) (X))
#define UINT64_AT(X) *((uint64_t*) (X))
#define WCHAR_AT(X)  (wchar_t) *((int16_t*) (X))

#define BYTE_TO_BINARY(byte) \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')

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
    [0x7F] = "DEL",
    ['\''] = "'\\''",
};

static void print_char(char ch)
{
    if (ch >= 0 && special_ascii[ch]) {
        printf("%s", special_ascii[ch]);
    } else if (IS_PRINTABLE(ch)) {
        printf("'%c'", ch);
    } else {
        printf("\ufffd");
    }
}

static void print_wchar(wchar_t wch)
{
    printf("'%lc'", wch);
    // TODO
}

static void print_utf8(char* bbb)
{
    char ch = bbb[0];
    // If ASCII character ....
    if ((ch & 0x80) == 0) {
        print_char(ch);
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

static void print_numbers(char buf[], bool le, int offset)
{
    pco(offset);
    printf(le ? "[LITTLE ENDIAN]" : "[BIG ENDIAN]");

    int8_t   i8  = INT8_AT(buf);
    int16_t  i16 = INT16_AT(buf);
    int32_t  i32 = INT32_AT(buf);
    uint8_t  u8  = UINT8_AT(buf);
    uint16_t u16 = UINT16_AT(buf);
    uint32_t u32 = UINT32_AT(buf);
    wchar_t  wch = WCHAR_AT(buf);

    if (le) {
        i16 = bswap_16(u16);
        i32 = bswap_32(u32);
        u16 = bswap_16(u16);
        u32 = bswap_32(u32);
        wch = bswap_16(wch);
    }

    pco(offset); printf("wchar: "); print_wchar(wch);

    pco(offset); printf("i8:  %11i 0x%hhX", i8,  i8);
    pco(offset); printf("i16: %11i 0x%hX",  i16, i16);
    pco(offset); printf("i32: %11i 0x%X",   i32, i32);
    pco(offset); printf("u8:  %11u 0x%hhX", u8,  u8);
    pco(offset); printf("u16: %11u 0x%hX",  u16, u16);
    pco(offset); printf("u32: %11u 0x%X",   u32, u32);
}

void chx_draw_data_inspector()
{
    // copy bytes from file
    char buf[5] = { 0 };
    for (int i = 0; i < 5 && CINST.cursor.pos + i < CINST.fdata.len; ++i) {
        buf[i] = CINST.fdata.data[CINST.cursor.pos + i];
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

    pco(offset);
    printf("bin: %c%c%c%c%c%c%c%c", BYTE_TO_BINARY(buf[0]));

    pco(offset); printf("char:  "); print_char(buf[0]);
    pco(offset); printf("utf-8: "); print_utf8(buf);

    pco(offset); // print LITTLE ENDIAN
    print_numbers(buf, true, offset);

    pco(offset); // print BIG ENDIAN
    print_numbers(buf, false, offset);

    printf(ANSI_RESET);
}
