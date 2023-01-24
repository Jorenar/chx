/* SPDX-License-Identifier:  MIT
 * Copyright 2021 nicholascok
 * Copyright 2023 Jorengarenar
 */

#include "interpreter.h"
#include "actions.h"

/* VOID INTERPRETER COMMANDS */
struct chx_void_command chx_void_commands[] = {
    (struct chx_void_command) { chx_print_finfo, "get-info" },
    (struct chx_void_command) { chx_toggle_inspector, "ti" },
    (struct chx_void_command) { chx_toggle_preview, "tp" },
    (struct chx_void_command) { chx_prompt_save_as, "saveas" },
    (struct chx_void_command) { chx_exit, "exit" },
    (struct chx_void_command) { chx_exit, "q!" },
    (struct chx_void_command) { chx_quit, "quit" },
    (struct chx_void_command) { chx_quit, "q" },
    (struct chx_void_command) { chx_save_and_quit, "wq" },
    (struct chx_void_command) { chx_to_start, "0" },
    (struct chx_void_command) { 0, 0 } // do not remove
};

/* INTERPRETER COMMANDS (WITH PARAMS) */
struct chx_command chx_commands[] = {
    (struct chx_command) { chx_set_inst, "to" },
    (struct chx_command) { chx_save_as, "save" },
    (struct chx_command) { chx_save_as, "sav" },
    (struct chx_command) { chx_save_as, "w" },
    (struct chx_command) { chx_config_layout_global, "gcfg" },
    (struct chx_command) { chx_config_layout, "cfg" },
    (struct chx_command) { chx_count_instances, "count" },
    (struct chx_command) { chx_count_instances, "cnt" },
    (struct chx_command) { chx_find_next, "find" },
    (struct chx_command) { chx_find_next, "f" },
    (struct chx_command) { chx_find_next, "/" },
    (struct chx_command) { chx_switch_file, "switch" },
    (struct chx_command) { chx_switch_file, "sw" },
    (struct chx_command) { chx_switch_file, "s" },
    (struct chx_command) { chx_open_instance, "open" },
    (struct chx_command) { chx_open_instance, "o" },
    (struct chx_command) { chx_close_instance, "close" },
    (struct chx_command) { chx_close_instance, "c" },
    (struct chx_command) { 0, 0 } // do not remove
};
