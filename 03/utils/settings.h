#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../global.c"
#include "file_operations.h"
#include "parson.h"
#include "utils.h"

char* settings_file_path;
int load_settings();
void free_settings();