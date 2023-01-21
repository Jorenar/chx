/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include <stdio.h>

#include "editor.h"
#include "utils.h"
#include "actions.h"

int main(int argc, char** argv)
{
    // allow printing of wide chars
    setlocale(LC_ALL, "");

    // command-line interface
    if (argc < 2) {
        printf("no filepath specified: see chx --help for more information.\n");
        return 0;
    } else {
        if (cmp_str(argv[1], "--help") || cmp_str(argv[1], "-help")) {
            printf("usage: chx <filepath>\n");
            return 0;
        } else if (cmp_str(argv[1], "-v") || cmp_str(argv[1], "--version")) {
            printf("CAOIMH HEX EDITOR version 1.0.1\n");
            return 0;
        }
    }

    // load file
    struct chx_finfo hdata = chx_import(argv[1]);
    hdata.filename = memfork(argv[1], str_len(argv[1]) + 1);
    if (!hdata.data) {
        printf("file \"%s\" not found.\n", argv[1]);
        return 1;
    }

    // enter new terminal state
    tenter();
    cls();

    // disable key echoing
    struct termios old = { 0 };
    tcgetattr(0, &old);
    old.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &old);

    // override ctrl+z/ctrl+c termination
    signal(SIGINT, chx_quit);
    signal(SIGTSTP, chx_quit);

    // setup initial instance
    CHX_CUR_MAX_INSTANCE = -1;
    CHX_SEL_INSTANCE = 0;
    CHX_INSTANCES = (struct CHX_INSTANCE*) calloc(sizeof (struct CHX_INSTANCE), CHX_MAX_NUM_INSTANCES);

    chx_add_instance(argv[1]);

    // only show preview or inspector if it will fit on the screen
    CINST.show_inspector = (CHX_PREVIEW_END + 28 > CINST.width) ? 0 : CHX_SHOW_INSPECTOR_ON_STARTUP;
    CINST.show_preview = (CHX_PREVIEW_END > CINST.width) ? 0 : CHX_SHOW_PREVIEW_ON_STARTUP;

    // if the screen cannot fit the contents, remove one byte until it can be displayed
    while (CHX_CONTENT_END > CINST.width && CINST.bytes_per_row) {
        CINST.bytes_in_group = 1;
        CINST.bytes_per_row--;
    }

    // alert user if application cannot be functionally operated
    if (CINST.num_rows < 2 || !CINST.bytes_per_row) {
        chx_exit_with_message("terminal is too small to run application.\n");
    }

    // draw elements
    chx_draw_all();

    // call main loop
    chx_main();

    texit();
    return 0;
}
