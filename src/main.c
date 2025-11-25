#include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "buildins.h"
#include "prompt.h"

#define CMD_SIZE 1024
#define MAX_TOKEN 100

//Global Variable
int token_count=0;
int current_token=0;

//prompt style
void prompt(){
    char* username=getlogin();
    char hostname[1024];
    char path[PATH_MAX];

    gethostname(hostname, sizeof(hostname));
    getcwd(path, sizeof(path));

    printf(YELLO"%s@",username);
    printf(BLUE"%s:~%s"BLUE, hostname,path);
    printf(BOLD"$ ");
    printf(RESET);
}

typedef enum{
    TOK_CMD,
    TOK_ARG,
    TOK_LOGIC,
    TOK_PIPE,
    TOK_SEQ,
    TOK_REDIR
} token_type;

typedef struct AST_node{
    token_type type;

    union{
        struct{
            char* cmd;
            struct AST_node* args;
            struct AST_node* redirect;
        } command;

        struct{
            char* arg;
            struct AST_node* nxt;
        } argument;

        struct {
            char* op;
            struct AST_node* left;
            struct AST_node* right;
        } connector;

        struct {
            char* type;
            char* destination;
        } redirect;
    } nodes;
} AST_node;

void create_node_command(AST_node** node, char* cmd){
    if(!node) return;
    
    (*node)->type=TOK_CMD;
    (*node)->nodes.command.cmd = strdup(cmd);
    (*node)->nodes.command.args=NULL;
    (*node)->nodes.command.redirect=NULL;
}

void create_node_argument(AST_node** node, char* arg){
    if(!node) return;
    
    (*node)->type=TOK_ARG;
    (*node)->nodes.argument.arg=strdup(arg);
    (*node)->nodes.argument.nxt=NULL;
}

AST_node* create_node_connector(char** operator, int type, AST_node* left, AST_node* right){
    AST_node* node = malloc(sizeof(AST_node));
    if(!node) return NULL;

    node->type = type;
    node->nodes.connector.op=strdup(*operator);
    node->nodes.connector.left=left;
    node->nodes.connector.right=right;

    return node;
}

void create_node_redirect(AST_node** node, char* op, char* destination){
    if(!node) return;

    (*node)->type=TOK_REDIR;
    (*node)->nodes.redirect.type=strdup(op);
    (*node)->nodes.redirect.destination=strdup(destination);
}

//function prototype
char* read_input();
int rm_space(char* input);
void get_token(char* input, char*** tokens);
bool is_connector(char* type);
bool is_redirect(char* type);
AST_node* parse_redirect(char*** tokens);
AST_node* parse_cmd_line(char*** tokens);
AST_node* parse_connector(AST_node* left, char*** tokens);
AST_node* parse_sim_cmd(char*** tokens);
AST_node* parse_cmd(char*** tokens);
AST_node* parse_arg(char*** tokens);
int execute_ast(AST_node* ast);
int exec_cmd(int size, char*** cmd_arr);
int exec_connetor(AST_node* node);
void free_ast(AST_node* ast);
//void print_ast(AST_node* ast);

int main(int argc, char** argv)
{
    while(1){
    prompt();
    char* input = read_input();
    if(!input){
        free(input);
        continue;
    }

    char** tokens=calloc(MAX_TOKEN, sizeof(char*));
    get_token(input, &tokens);
    if(token_count==0){
        free(input);
        continue;
    }

    AST_node* ast=parse_cmd_line(&tokens);
    int stat = execute_ast(ast);
    //print_ast(ast);

    exit_stat=stat;
    free_ast(ast);
    free(input);
    for(int i=0; i<token_count; i++){
        free(tokens[i]);
    }
    free(tokens);
    token_count=0;
    current_token=0;
    }

    return 0;
}

char* read_input(){
    char* cmd=malloc(CMD_SIZE*sizeof(char));
    int index=0;
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

int rm_space(char* input){
    int pos=0;
    char* ptr=input;
    while(*(ptr)!='\0' && *(ptr) == ' '){
        pos++;
        ptr++;
    }
    return pos;
}

void get_token(char* input, char*** tokens)
{
    int i=rm_space(input);
    int j;
    char quote_char;

    while(input[i]!='\0'){
        j=0;
        char* token=malloc(256);

        while(input[i]!='\0' && input[i]==' '){
            i++;
        }
        if(input[i]=='\0'){
            free(token);
            break;
        }
        
        if(input[i]=='"' || input[i]=='\''){
            quote_char=input[i];
            i++;
            while(input[i]!=quote_char && input[i]){
                token[j++]=input[i++];
            }

            if(input[i]==quote_char)i++;
            token[j]='\0';
        }
        else if( strchr("><|;&", input[i])!=NULL ){
            if(input[i+1]!='\0' && input[i]==input[i+1] && (input[i]=='|' || input[i]=='&' || input[i]=='>'))
            {
                token[j++]=input[i++];
                token[j++]=input[i++];
                token[j]='\0';
            }
            else
            {
                token[j++]=input[i++];
                token[j]='\0';
            }
        }else{
            while( input[i]!=' ' && input[i] && strchr("><|;&", input[i])==NULL ){
                token[j++]=input[i++];
            }
            token[j]='\0';
        }
        
        if(token != NULL){
            (*tokens)[token_count] = strdup(token);
        }
        token_count++;

        free(token);
    }
    (*tokens)[token_count] = NULL;
}

bool is_connector(char* type){
    if(type == NULL) return false;
    if(strcmp(type, "||")==0 || strcmp(type, "&&")==0 || strcmp(type, "|")==0 || strcmp(type, ";")==0){
        return true;
    }
    return false;
}

bool is_redirect(char* type){
    if(type == NULL) return false;
    if(strcmp(type, ">>")==0 || strcmp(type, ">")==0){
        return true;
    }

    return false;
}

AST_node* parse_cmd_line(char*** tokens){
    AST_node* left = parse_cmd(&(*tokens));
    if(!left)
        return NULL;

    return parse_connector(left, &(*tokens));
}

AST_node* parse_cmd(char*** tokens){
    AST_node* cmd = parse_sim_cmd(&(*tokens));
    if(!cmd){
        return NULL;
    }

    cmd->nodes.command.args=parse_arg(&(*tokens));

    if(current_token<token_count && is_redirect((*tokens)[current_token])){
        cmd->nodes.command.redirect=parse_redirect(&(*tokens));
    }

    return cmd;
}

AST_node* parse_sim_cmd(char*** tokens){
    if(current_token>=token_count){
        return NULL;
    }

    if(is_connector((*tokens)[current_token]) || is_redirect((*tokens)[current_token])){
        return NULL;
    }

    AST_node* cmd=malloc(sizeof(AST_node));
    create_node_command(&cmd, (*tokens)[current_token]);
    current_token++;

    return cmd;
}

AST_node* parse_arg(char*** tokens){
    if(current_token>=token_count){
        return NULL;
    }

    if(is_connector((*tokens)[current_token]) || is_redirect((*tokens)[current_token])){
        return NULL;
    }

    AST_node* arg=malloc(sizeof(AST_node));
    create_node_argument(&arg, (*tokens)[current_token]);
    current_token++;
    arg->nodes.argument.nxt=parse_arg(&(*tokens));

    return arg;
}

AST_node* parse_redirect(char*** tokens){
    if(current_token>=token_count || !is_redirect((*tokens)[current_token])){
        return NULL;
    }

    char* redirect_type=strdup((*tokens)[current_token]);
    current_token++;

    if(current_token>=token_count){
        fprintf(stderr, "Syntax Error: Expected destination\n");
        return NULL;
    }
    char* destin=strdup((*tokens)[current_token]);
    current_token++;

    AST_node* redir=malloc(sizeof(AST_node));
    create_node_redirect(&redir, redirect_type, destin);

    free(redirect_type);
    free(destin);
    return redir;
}

AST_node* parse_connector(AST_node* left, char*** tokens){
    while(current_token<token_count && is_connector((*tokens)[current_token])){
        char* op=strdup((*tokens)[current_token]);
        if(op!=NULL) current_token++;

        int type;
        if(strcmp(op, "|")==0){
            type=TOK_PIPE;
        }else if(strcmp(op, ";")==0){
            type=TOK_SEQ;
        }else if(strcmp(op, "||")==0 || strcmp(op, "&&")==0){
            type=TOK_LOGIC;
        }

        AST_node* right=parse_cmd(&(*tokens));
        if(!right){
            return NULL;
        }

        left = create_node_connector(&op, type, left, right);
        free(op);
    }

    return left;
}

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

        if(exec_cmd(i, &cmd_arr)>0){
            for(int k=0; k<i; k++) free(cmd_arr[k]);
            free(cmd_arr);
            return 1;
        }

        for(int k=0; k<i; k++) free(cmd_arr[k]);
        free(cmd_arr);
    }else if(type==TOK_LOGIC || type==TOK_SEQ){
        exec_connetor(ast);
    }else{
        fprintf(stderr, "%s\n", "Unknown Command");
        return 1;
    }

    return 0;
}

int exec_cmd(int size, char*** cmd_arr){
    if(size<=0 || (*cmd_arr) == NULL || (*cmd_arr)[0] == NULL) return 1;

    cmd_func* ptr = cmd_func_arr;
    
    for(int i=0; i<total_buildins && ptr[i].cmd_name != NULL; ++i){
        if(strcmp((*cmd_arr)[0], ptr[i].cmd_name) == 0){
            return ptr[i].cmd_func(size, (*cmd_arr));
        }
    }

    int status;
    pid_t pid=fork();
    if(pid<0){
        perror("fork");
        _exit(1);
    }

    if(pid==0){
        if(execvp((*cmd_arr)[0], (*cmd_arr))==-1){
            perror((*cmd_arr)[0]);
            return 1;
        }
    }

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
    int stat;
        
    if(strcmp(node->nodes.connector.op, ";")==0){
        execute_ast(left);
        execute_ast(right);
    }else if(strcmp(node->nodes.connector.op, "||")==0){
        stat = execute_ast(left);
        if(stat==1)
            if(execute_ast(right)==1) return 1;
    }else if(strcmp(node->nodes.connector.op, "&&")==0){
        stat = execute_ast(left);
        if(stat==0) 
            if(execute_ast(right)==1) return 1;
    }

    return 0;
}

void free_ast(AST_node* ast){
    if(!ast) return;

    switch(ast->type){
        case TOK_CMD:
            if(ast->nodes.command.cmd) free(ast->nodes.command.cmd);

            if(ast->nodes.command.args) free_ast(ast->nodes.command.args);

            if(ast->nodes.command.redirect) free_ast(ast->nodes.command.redirect);
        
            break;

        case TOK_ARG:
            if(ast->nodes.argument.arg) free(ast->nodes.argument.arg);

            if(ast->nodes.argument.nxt) free_ast(ast->nodes.argument.nxt);

            break;
        
        case TOK_REDIR:
            if(ast->nodes.redirect.type) free(ast->nodes.redirect.type);
            
            if(ast->nodes.redirect.destination) free(ast->nodes.redirect.destination);

            break;
        
        case TOK_LOGIC:
        case TOK_PIPE:
        case TOK_SEQ:
            if(ast->nodes.connector.op) free(ast->nodes.connector.op);

            if(ast->nodes.connector.left) free_ast(ast->nodes.connector.left);

            if(ast->nodes.connector.right) free_ast(ast->nodes.connector.right);

            break;
        default:
            break;
    }
    free(ast);
}

void print_ast(AST_node* ast){
    if(!ast) return;

    switch(ast->type){
        case TOK_CMD:
            if(ast->nodes.command.cmd) printf("%s\n", ast->nodes.command.cmd);

            if(ast->nodes.command.args) print_ast(ast->nodes.command.args);

            if(ast->nodes.command.redirect) print_ast(ast->nodes.command.redirect);
        
            break;

        case TOK_ARG:
            if(ast->nodes.argument.arg) printf("%s\n", ast->nodes.argument.arg);

            if(ast->nodes.argument.nxt) print_ast(ast->nodes.argument.nxt);

            break;
        
        case TOK_REDIR:
            if(ast->nodes.redirect.type) printf("%s\n", ast->nodes.redirect.type);
            
            if(ast->nodes.redirect.destination) printf("%s\n", ast->nodes.redirect.destination);

            break;
        
        case TOK_LOGIC:
        case TOK_PIPE:
        case TOK_SEQ:
            if(ast->nodes.connector.op)
            printf("%s\n", ast->nodes.connector.op);

            if(ast->nodes.connector.left){
                printf("[L] ");
                print_ast(ast->nodes.connector.left);
            }else{
                perror("Syntax error");
                return;
            }

            if(ast->nodes.connector.right){
                printf("[R] ");
                print_ast(ast->nodes.connector.right);
            }else{
                perror("Syntax error");
                return;
            }

            break;
        
        default:
            perror("Unknown command\n");
            break;
    }
}
