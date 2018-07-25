#ifndef _CONFIG_MIKES_H_
#define _CONFIG_MIKES_H_

#include "config/config.h"

#define MIKES_CONFIG "mikes-common-tests.cfg"

typedef struct {
    int autostart;
    int with_gui;
    int print_all_logs_to_console;
    int print_debug_logs;
    int use_ncurses_control;
    int start_x;
    int start_y;
} mikes_config_t;

extern mikes_config_t mikes_config;

void load_config();

#endif
