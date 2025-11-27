#ifndef BUILDINS_H
#define BUILDINS_H

#include "headers.h"

typedef int (*bi_func)(int argc, char** argv);

typedef struct cmd_func{
    const char* cmd_name;
    bi_func cmd_func;
} cmd_func;

extern cmd_func cmd_func_arr[];
extern int total_buildins;
extern int exit_stat;

#endif //BUILDINS_H