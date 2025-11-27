#include "prompt.h"

//prompt style
void prompt(){
    char* username=getlogin();
    char hostname[1024];

    gethostname(hostname, sizeof(hostname));

    printf(WHITE"%s"RESET,username);
    printf(CYAN"@%s:"RESET, hostname);
    printf(BOLD"$ "RESET);
}

char* read_input(){
    char* cmd=malloc(CMD_SIZE*sizeof(char));
    unsigned int index=0;
    unsigned int size=CMD_SIZE;

    //handling error in memory allocation
    if(cmd==NULL){
        perror("allocation failed");
        return NULL;
    }

    while(1){
        int ch=getchar();
        
        if(ch==EOF || ch=='\n'){
            cmd[index]='\0';
            return cmd;
        }else{
            cmd[index]=(char)ch;
        }
        index++;

        if(index>=size){
            size+=CMD_SIZE;
            cmd = realloc(cmd, sizeof(char)*size);

            if(!cmd){
                perror("allocation failed");
                return NULL;
            }
        }
    }

    return cmd;
}
