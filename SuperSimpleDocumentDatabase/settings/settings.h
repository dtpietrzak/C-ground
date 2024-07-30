#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sdb_utils.h>

#include "../global.c"

char* settings_file_path;
int load_settings();
void free_settings();

#endif  // SETTINGS_H