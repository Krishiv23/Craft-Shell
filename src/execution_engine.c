#include "execution_engine.h"

int execute_ast(AST_node* ast){
    if(!ast) return 1;

    int type = ast->type, i=0;

    if(type == TOK_CMD){
        char** cmd_arr = malloc(MAX_TOKEN*sizeof(char*));
        if(!cmd_arr) return 1;
        char* flag = malloc(100);
        if(!flag) return 1;
        cmd_arr[i++] = strdup(ast->nodes.command.cmd);
        if(cmd_arr[i-1]==NULL) return 1;
        AST_node* temp = ast->nodes.command.args;
        int j=0;
        flag[j++]='-';

        while(temp){
            char* arg = temp->nodes.argument.arg;
            if(*arg == '-'){
                if(strcmp(cmd_arr[0],"cd")==0){
                    cmd_arr[i++]=strdup(arg);
                    break;
                }
                arg++;
                while((*arg)!='\0'){
                    flag[j++]=*(arg);
                    arg++;
                }
            }else{
                break;
            }

            temp = temp->nodes.argument.nxt;
        }
        flag[j]='\0';

        if(j>=2 && flag[1]!='\0'){
            cmd_arr[i++] = strdup(flag);
        }

        free(flag);
        flag=NULL;

        while(temp){
            cmd_arr[i++]=strdup(temp->nodes.argument.arg);
            temp = temp->nodes.argument.nxt;
        }
        cmd_arr[i]=NULL;

        if(exec_cmd(i, &cmd_arr, ast->nodes.command.redirect)>0 || if_exit==1){
            for(int k=0; k<i; k++) free(cmd_arr[k]);
            free(cmd_arr);
            return 1;
        }

        for(int k=0; k<i; k++) free(cmd_arr[k]);
        free(cmd_arr);
    }else if(type==TOK_LOGIC || type==TOK_SEQ || type==TOK_PIPE){
        exec_connetor(ast);
    }else{
        fprintf(stderr, "%s\n", "Unknown Command");
        return 1;
    }

    return 0;
}

int exec_cmd(int size, char*** cmd_arr, AST_node* redir){
    if(size<=0 || (*cmd_arr) == NULL || (*cmd_arr)[0] == NULL) return 1;

    cmd_func* ptr = cmd_func_arr;
    
    for(int i=0; i<total_buildins && ptr[i].cmd_name != NULL; ++i){
        int return_val;
        int s_out=-1;
        int s_in=-1;
        if(strcmp((*cmd_arr)[0], ptr[i].cmd_name) == 0){

            if(redir && redir->type==TOK_REDIR){
                int fd;
                char* destin = redir->nodes.redirect.destination;
                char* type = redir->nodes.redirect.type;

                if(strcmp(type, "<")==0){
                    s_in = dup(STDIN_FILENO);
                    if(s_in < 0){
                        perror("dup stdin");
                        return 1;
                    }

                    fd=open(destin, O_RDONLY, 0644);
                    if(fd<0){
                        perror("open input");
                        return 1;
                    }

                    if(dup2(fd, STDIN_FILENO)<0){
                        perror("dup2 stdin");
                        close(s_in);
                        close(fd);
                        return 1;
                    }
                    close(fd);

                }else if(strcmp(type, ">")==0){
                    s_out = dup(STDOUT_FILENO);
                    if(s_out < 0){
                        perror("dup stdout");
                        return 1;
                    }

                    fd = open(destin, O_CREAT | O_RDWR | O_TRUNC, 0644);
                    if(fd<0){
                        perror("open output file");
                        return 1;
                    }

                    if(dup2(fd, STDOUT_FILENO)<0){
                        perror("dup2 stdout");
                        close(fd);
                        close(s_out);
                        return 1;
                    }

                    close(fd);
                }else if(strcmp(type, ">>")==0){
                    s_out = dup(STDOUT_FILENO);
                    if(s_out<0){
                        perror("dup stdout");
                        return 1;
                    }

                    fd = open(destin, O_CREAT | O_RDWR | O_APPEND, 0644);
                    if(fd<0){
                        perror("open output file");
                        return 1;
                    }

                    if(dup2(fd, STDOUT_FILENO)<0){
                        perror("dup2 stdout");
                        close(fd);
                        close(s_out);
                        return 1;
                    }

                    close(fd);
                }
            }
            
            return_val = ptr[i].cmd_func(size, (*cmd_arr));

            //Restoring fds
            if(s_in!=-1){
                dup2(s_in, STDIN_FILENO);
                close(s_in);
            }
            if(s_out!=-1){
                dup2(s_out, STDOUT_FILENO);
                close(s_out);
            }

            return return_val;
        }
    }

    pid_t pid=fork();
    if(pid<0){
        perror("fork");
        _exit(1);
    }

    if(pid==0){
        if(redir && redir->type==TOK_REDIR){
            int fd;
            char* destin = redir->nodes.redirect.destination;
            char* type = redir->nodes.redirect.type;

            if(strcmp(type, "<")==0){
                fd = open(destin, O_RDONLY, 0644);
                if(fd<0){
                    perror("open");
                    _exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }else if(strcmp(type, ">")==0){
                fd = open(destin, O_RDWR | O_CREAT | O_TRUNC, 0644);
                if(fd<0){
                    perror("open");
                    _exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }else if(strcmp(type, ">>")==0){
                fd = open(destin, O_RDWR | O_CREAT | O_APPEND, 0644);
                if(fd<1){
                    perror("open");
                    _exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        }

        if(execvp((*cmd_arr)[0], (*cmd_arr))==-1){
            perror((*cmd_arr)[0]);
            return 1;
        }
    }

    int status;
    pid = waitpid(pid, &status, 0);
    if(WIFEXITED(status) && WEXITSTATUS(status)==127){
        fprintf(stderr, "Command not found\n");
        return 1;
    }

    return 0;
}

int exec_connetor(AST_node* node){
    AST_node* left = node->nodes.connector.left;
    AST_node* right = node->nodes.connector.right;
    int stat=0;
        
    if(strcmp(node->nodes.connector.op, ";")==0){
        execute_ast(left);
        stat = execute_ast(right);
    }else if(strcmp(node->nodes.connector.op, "||")==0){
        stat = execute_ast(left);
        if(stat==1) stat = execute_ast(right);
    }else if(strcmp(node->nodes.connector.op, "&&")==0){
        stat = execute_ast(left);
        if(stat==0) stat = execute_ast(right);
    }else if(strcmp(node->nodes.connector.op, "|")==0){
        stat = exec_pipe(node->nodes.connector.left, node->nodes.connector.right);
    }

    return stat;
}

int exec_pipe(AST_node* left, AST_node* right){
    if(!left || !right) return 1;

    int fd[2];
    if(pipe(fd)==-1){
        perror("pipe");
        return 1;
    }

    pid_t pid_left = fork();
    if(pid_left==-1){
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if(pid_left==0){
        close(fd[0]);//readend
        if(dup2(fd[1], STDOUT_FILENO) == -1){
            perror("dup2 left stdout");
            _exit(1);
        }
        close(fd[1]);

        int left_stat=execute_ast(left);
        _exit(left_stat);
    }

    //parent
    close(fd[1]);

    pid_t pid_right = fork();
    if(pid_right==-1){
        perror("fork");
        close(fd[0]);

        int status;
        waitpid(pid_left, &status, 0);
        return 1;
    }
    if(pid_right==0){
        close(fd[1]);
        if(dup2(fd[0], STDIN_FILENO)==-1){
            perror("dup2 right stdin");
            _exit(1);
        }
        close(fd[0]);

        int right_stat=execute_ast(right);
        _exit(right_stat);
    }
    close(fd[0]);

    int s_left, s_right;
    waitpid(pid_left, &s_left, 0);
    waitpid(pid_right, &s_right, 0);

    if(WIFEXITED(s_right)) return WEXITSTATUS(s_right);
    
    return 1;
}