#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "buildins.h"

int bi_cd(int argc, char** argv){
    return 0;
}

int bi_echo(int argc, char** argv){
    return 0;
}

int bi_exit(int argc, char** argv){
    return 0;
}

int bi_pwd(int argc, char** argv){
    char path[1024];
    getcwd(path, sizeof(path));

    if(fprintf(stdout, "%s\n", path) < 0) return 1;

    return 0;
}

int bi_ls(int argc, char** argv){
    return 0;
}

cmd_func cmd_func_arr[] = {
    {"cd", bi_cd},
    {"echo", bi_echo},
    {"exit", bi_exit},
    {"pwd", bi_pwd},
    {"ls", bi_ls},
    {NULL, NULL}
};

int total_buildins = sizeof(cmd_func_arr)/sizeof(cmd_func_arr[0]);
