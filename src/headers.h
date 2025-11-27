#ifndef HEADER_H
#define HEADER_H

#define UNUSED __attribute_maybe_unused__
#define MAX_TOKEN 100
#define CMD_SIZE 1024
#define PATH_MAX 4096

extern int token_count;
extern int current_token;
extern int if_exit;

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif //HEADER_H