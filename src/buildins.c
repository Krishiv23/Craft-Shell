#include "buildins.h"
#include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

char last_dir[PATH_MAX]; //initialized to 0
int exit_stat;

int bi_cd(int argc, char** argv){
    char cur_dir[PATH_MAX];
    char path[PATH_MAX];

    if(!getcwd(cur_dir, sizeof(cur_dir))) *cur_dir = '\0';
    if(argv[1]==NULL && argc<=2){
        strcpy(path, getenv("HOME"));
        if(argv[1]==NULL){ 
            fprintf(stderr,"Home is not set");
            return 1;
        }
    }else if(strcmp(argv[1], "-")==0){
        if(*last_dir==0){
            fprintf(stderr, "No previous directory");
            return 1;
        }
        strcpy(path, last_dir);
    }else{
        if(argv[1][0] == '~'){
            if(argv[1][1]=='/' || argv[1][1]=='\0'){
                char* home=getenv("HOME");
                if(!home){
                    fprintf(stderr,"Home is not set");
                    return 1;
                }

                snprintf(path, sizeof(path), "%s%s", home, ((argv[1])+1));
            }else{
                fprintf(stderr, "cd: %s","no such file or directory");
                return 1;
            }
        }
    }

    if(chdir(path)!=0){
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    strcpy(last_dir, cur_dir);

    return 0;
}

int bi_echo(int argc, char** argv){
    if(argv[1]==NULL) fprintf(stdout, "\n");
    else fprintf(stdout, "%s\n", argv[1]);
    return 0;
}

int bi_exit(int argc, char** argv){
    if(argc>2){
        fprintf(stderr, "exit: %s", "Too many arguments");
        return 1;
    }

    if(argv[1]!=NULL){
        if(isdigit(argv[1][0]-'0')){
            exit_stat = argv[1][0]-'0';
            exit(exit_stat);
        }
    }
    exit(exit_stat);
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
