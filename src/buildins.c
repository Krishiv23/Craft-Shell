#include "buildins.h"

char last_dir[PATH_MAX]; //initialized to 0
int exit_stat;

int bi_cd(int argc, char** argv){
    char cur_dir[PATH_MAX]={0};
    char path[PATH_MAX]={0};

    if(argc>3){
        fprintf(stderr, "Too many arguments\n");
        return 1;
    }

    if(!getcwd(cur_dir, sizeof(cur_dir))) *cur_dir = '\0';

    if(argv[1]==NULL && argc<=2){
        strcpy(path, getenv("HOME"));
        if(*path == 0){ 
            fprintf(stderr,"Home is not set\n");
            return 1;
        }
    }else if(strcmp(argv[1], "-")==0){
        printf("%s\n", last_dir);
        if(*last_dir==0){
            fprintf(stderr, "No previous directory\n");
            return 1;
        }
        strcpy(path, last_dir);
    }else{
        if(argv[1][0] == '~'){
            if(argv[1][1]=='/' || argv[1][1]=='\0'){
                char* home=getenv("HOME");
                if(!home){
                    fprintf(stderr,"Home is not set\n");
                    return 1;
                }

                snprintf(path, sizeof(path), "%s%s", home, ((argv[1])+1));
            }
        }else if(strcmp(argv[1], "..")==0){
            strcpy(path, argv[1]);
        }else if(argv[1][0] == '.'){
            if(argv[1][1]=='/' || argv[1][1]=='\0'){
                snprintf(path, sizeof(path), "%s%s", cur_dir, ((argv[1])+1));
            }
        }else{
            fprintf(stderr, "cd: no such file or directory --%s\n", argv[1]);
            return 1;
        }
    }

    if(chdir(path)!=0){
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    strcpy(last_dir, cur_dir);

    return 0;
}

int bi_echo(int UNUSED argc, char** argv){
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
        if(isdigit((unsigned char)argv[1][0])){
            exit_stat = atoi(argv[1]);
        }
    }

    if_exit = 1;

    return exit_stat;
}

int bi_pwd(int UNUSED argc, char** UNUSED argv){
    char path[1024];
    getcwd(path, sizeof(path));

    if(fprintf(stdout, "%s\n", path) < 0) return 1;

    return 0;
}

int bi_ls(int argc, char** argv){
    DIR* dir;
    struct dirent* entry;

    char flag[100]={0};
    char path[PATH_MAX]={0};

    if(argc>3){
        fprintf(stderr, "Too many arguments");
        return 1;
    }

    for(int i=1; i<argc; ++i){
        if(argv[i][0]=='-'){
            strcpy(flag, argv[i]);
        }else{
            strcpy(path, argv[i]);
        }
    }

    if(*flag!=0 && strcmp(flag, "-a")!=0){
        fprintf(stderr, "Invalid option -%s\n", flag);
        return 1;
    }
    
    if(*path != 0){
        dir=opendir(path);
    }else{
        strcpy(path, ".");
        dir=opendir(path);
    }

    if(!dir){
        fprintf(stderr, "ls: cannot access '%s'\n", path);
        return 1;
    }

    if(*flag != 0){
        if(flag[1]=='a'){
            while((entry = readdir(dir))!=NULL){
                fprintf(stdout, "%s\n", entry->d_name);
            }
        }else{
            fprintf(stderr, "ls: invalid option --%c\n", flag[1]);
            return 1;
        }
    }else{
        while((entry=readdir(dir))!=NULL){
            if(*(entry->d_name) != '.'){
                fprintf(stdout, "%s\n", entry->d_name);
            }
        }
    }

    if(closedir(dir)!=0){
        fprintf(stderr, "error closing the directory\n");
        return 1;
    }
    return 0;
}

int bi_new(int argc, char** argv){
    if(argc==1){
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }
    char path[PATH_MAX]={0};
    int fd=-1, u=6, g=4, o=4;

    for(int i=1; i<argc; ++i){
        if(argv[i][0]=='-'){
            int len = strlen(argv[i]);
            for(int j=1; j<len; ++j){
                switch(argv[i][j]){
                    case 'u':
                        if(argv[i][j+1]!='\0' && isdigit((unsigned char)argv[i][j+1])){
                            if(strchr("01234567", argv[i][++j])){
                                u = argv[i][j]-'0';
                            }else{
                                fprintf(stderr, "Invalid Permission -- %c", argv[i][j]);
                                return 1;
                            }
                        }
                    break;
                    case 'g':
                        if(argv[i][j+1]!='\0' && isdigit((unsigned char)argv[i][j+1])){
                            if(strchr("01234567", argv[i][++j])){
                                g = argv[i][j]-'0';
                            }else{
                                fprintf(stderr, "Invalid Permission-- %c", argv[i][j]);
                                return 1;
                            }
                        }
                    break;
                    case 'o':
                        if(argv[i][j+1]!='\0' && isdigit((unsigned char)argv[i][j+1])){
                            if(strchr("01234567", argv[i][++j])){
                                o = argv[i][j]-'0';
                            }else{
                                fprintf(stderr, "Invalid Permission-- %c", argv[i][j]);
                                return 1;
                            }
                        }
                    break;
                    default:
                        fprintf(stderr, "Invalid Option %s", argv[i]);
                        return 1;
                }
            }
        }else{
            strcpy(path, argv[i]);
        }
    }

    if(*path != 0)
    fd = open(path, O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    if(fd<0){
        perror("Error opening file\n");
        return 1;
    }

    struct stat fs;
    if(stat(path, &fs)<0){
        perror("stat");
        close(fd);
        return 1;
    }

    mode_t new_mode = fs.st_mode & ~(S_IRWXU | S_IRWXG | S_IRWXO);

    if(u & 4)
        new_mode |= S_IRUSR;
    if(u & 2)
        new_mode |= S_IWUSR;
    if(u & 1)
        new_mode |= S_IXUSR;

    if(g & 4)  
        new_mode |= S_IRGRP;
    if(g & 2)
        new_mode |= S_IWGRP;
    if(g & 1)
        new_mode |= S_IXGRP;

    if(o & 4) new_mode |= S_IROTH;
    if(o & 2) new_mode |= S_IWOTH;
    if(o & 1) new_mode |= S_IXOTH;

    if(chmod(path, new_mode)<0){
        perror("Error setting  permission");
        close(fd);
        return 1;
    }

    if(close(fd)!=0){
        fprintf(stderr, "Error closing file\n");
        return 1;
    }

    return 0;
}

int bi_cat(int argc, char** argv){
    char buffer[2048];
    int fd, count;
   
    for(int i=1; i<argc; i++){
         if(strcmp(argv[i],"-")==0 || argv[i]==NULL){
            fd = STDIN_FILENO;
        }else{
            fd = open(argv[i], O_RDONLY, 0);
            if(fd<0){
                perror("open");
                return 1;
            }
        }

        while((count=read(fd, buffer, sizeof(buffer)))>0){
            if(write(STDOUT_FILENO, buffer, count) != count){
                perror("write");
                if(fd>2) close(fd);
                return 1;
            }
        }
        if(count<0){
            perror("read");
            if(fd>2) close(fd);
            return 1;
        }
        if(fd>2) close(fd);
    }
    putchar('\n');

    return 0;
}

int bi_help(int UNUSED argc, char** UNUSED argv){
    cmd_func* ptr = cmd_func_arr;
    fprintf(stdout, "Available built-in commands:\n");
    for(int i=0; i<total_buildins && ptr[i].cmd_name != NULL; ++i){
        fprintf(stdout, "\t%s\n", ptr[i].cmd_name);
    }
    return 0;
}

cmd_func cmd_func_arr[] = {
    {"cd", bi_cd},
    {"echo", bi_echo},
    {"exit", bi_exit},
    {"help", bi_help},
    {"pwd", bi_pwd},
    {"ls", bi_ls},
    {"new", bi_new},
    {"cat", bi_cat},
    {NULL, NULL}
};

int total_buildins = sizeof(cmd_func_arr)/sizeof(cmd_func_arr[0]);
