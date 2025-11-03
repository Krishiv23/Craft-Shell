#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include "prompt.h"

#define CMD_SIZE 1024

//Global Variable
int token_count=0;
char** tokens;
int current_token=0;

//prompt style
void prompt(){
    char* username=getlogin();
    char hostname[1024];
    char path[1024];

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

AST_node* create_node_command(char* cmd){
    AST_node* node=malloc(sizeof(AST_node));
    if(!node)
        return NULL;
    
    node->type=TOK_CMD;
    node->nodes.command.cmd = strdup(cmd);
    node->nodes.command.args=NULL;
    node->nodes.command.redirect=NULL;

    return node;
}

AST_node* create_node_argument(char* arg){
    AST_node* node=malloc(sizeof(AST_node));
    if(!node)
        return NULL;
    
    node->type=TOK_ARG;
    node->nodes.argument.arg=strdup(arg);
    node->nodes.argument.nxt=NULL;

    return node;
}

AST_node* create_node_connector(char* operator, AST_node* left, AST_node* right){
    AST_node* node=malloc(sizeof(AST_node));
    if(!node)
        return NULL;
    
    if(strcmp(operator, "|")==0){
        node->type=TOK_PIPE;
    }else if(strcmp(operator, ";")==0){
        node->type=TOK_SEQ;
    }else if(strcmp(operator, "||")==0 || (strcmp(operator, "&&")==0)){
        node->type=TOK_LOGIC;
    }

    node->nodes.connector.op=strdup(operator);
    node->nodes.connector.left=left;
    node->nodes.connector.right=right;

    return node;
}

AST_node* create_node_redirect(char* type, char* destination){
    AST_node* node=malloc(sizeof(AST_node));
    if(!node)
        return NULL;

    node->type=TOK_REDIR;
    node->nodes.redirect.type=strdup(type);
    node->nodes.redirect.destination=strdup(destination);

    return node;
}

//function prototype
char* read_input();
int rm_space(char* input);
char** get_token(char* input, int *token_count);
bool is_connector(char* type);
bool is_redirect(char* type);
AST_node* parse_redirect();
AST_node* parse_cmd_line();
AST_node* parse_connector(AST_node* left);
AST_node* parse_sim_cmd();
AST_node* parse_cmd();
AST_node* parse_arg();
void print_ast(AST_node* ast);

int main(int argc, char** argv)
{
    while(1){
    prompt();
    char* input = read_input();
    if(!input){
        free(input);
        continue;
    }
    tokens = get_token(input, &token_count);
    if(token_count==0){
        free(input);
        continue;
    }

    AST_node* ast=parse_cmd_line();
    print_ast(ast);

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

int rm_space(char* input)
{
    int pos=0;
    while(input[pos]==' '){
        pos++;
    }
    return pos;
}

char** get_token(char* input, int *token_count)
{
    int i=rm_space(input);
    int j, a, b;
    char quote_char;

    *token_count=0;

    char** tokens=malloc(100*sizeof(char*));
    for(int x=0; x<100; x++){
        tokens[x]=malloc(256);
    }

    while(input[i]){
        j=0, a=0, b=0;
        char* token=malloc(256);

        while(input[i]==' ' && input[i]){
            i++;
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
            if(input[i+1] && input[i]==input[i+1] && (input[i]=='|' || input[i]=='&' || input[i]=='>'))
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
        }
        else{
            while(input[i]!=' ' && input[i] && strchr("><|;&", input[i])==NULL ){
                token[j++]=input[i++];
            }
            token[j]='\0';
        }
        
        while(token[b]){
            tokens[(*token_count)][a++]=token[b++];
        }
        tokens[*token_count][a]='\0';
        (*token_count)++;

        free(token);
    }
    return tokens;
}

bool is_connector(char* type){
    if(type == NULL) return false;
    if(strcmp(type, "|")==0 | strcmp(type, "&&")==0 | strcmp(type, "||")==0 | strcmp(type, ";")==0){
        return true;
    }
    return false;
}

bool is_redirect(char* type){
    if(type == NULL) return false;
    if(strcmp(type, ">")==0 || strcmp(type, ">>")==0){
        return true;
    }

    return false;
}

AST_node* parse_cmd_line(){
    AST_node* left = parse_cmd();
    if(!left)
        return NULL;

    return parse_connector(left);
}

AST_node* parse_cmd(){
    AST_node* cmd = parse_sim_cmd();
    if(!cmd){
        return NULL;
    }

    cmd->nodes.command.args=parse_arg();

    if(current_token<token_count && is_redirect(tokens[current_token])){
        cmd->nodes.command.redirect=parse_redirect();
    }

    return cmd;
}

AST_node* parse_connector(AST_node* left){
    while(current_token<token_count && is_connector(tokens[current_token])){
        char* op=tokens[current_token];
        current_token++;

        AST_node* right=parse_cmd();
        if(!right){
            return NULL;
        }

        left = create_node_connector(op, left, right);
    }

    return left;
}

AST_node* parse_sim_cmd(){
    if(current_token>token_count){
        return NULL;
    }

    if(is_connector(tokens[current_token]) || is_redirect(tokens[current_token])){
        return NULL;
    }

    AST_node* cmd=create_node_command(tokens[current_token]);
    current_token++;

    return cmd;
}

AST_node* parse_arg(){
    if(current_token>token_count){
        return NULL;
    }

    if(is_connector(tokens[current_token]) || is_redirect(tokens[current_token])){
        return NULL;
    }

    AST_node* arg=create_node_argument(tokens[current_token]);
    current_token++;
    arg->nodes.argument.nxt=parse_arg();

    return arg;
}

AST_node* parse_redirect(){
    if(current_token>=token_count || !is_redirect(tokens[current_token])){
        return NULL;
    }

    char* redirect_type=strdup(tokens[current_token]);
    current_token++;

    if(current_token>=token_count){
        perror("Syntax Error: Expected destination\n");
        return NULL;
    }
    char* destin=strdup(tokens[current_token]);
    current_token++;

    AST_node* redir=create_node_redirect(redirect_type, destin);

    return redir;
}
void print_ast(AST_node* ast){
    if(!ast) return;

    switch(ast->type){
        case TOK_CMD:
            if(ast->nodes.command.cmd)
            printf("%s\n", ast->nodes.command.cmd);

            if(ast->nodes.command.args){
                print_ast(ast->nodes.command.args);
            }

            if(ast->nodes.command.redirect){
                print_ast(ast->nodes.command.redirect);
            }
        
            break;
        
        case TOK_ARG:
            if(ast->nodes.argument.arg)
            printf("%s\n", ast->nodes.argument.arg);

            if(ast->nodes.argument.nxt)
            print_ast(ast->nodes.argument.nxt);

            break;
        
        case TOK_REDIR:
            if(ast->nodes.redirect.type)
            printf("%s ", ast->nodes.redirect.type);

            if(ast->nodes.redirect.destination)
            printf("%s\n", ast->nodes.redirect.destination);

            break;

        case TOK_PIPE:
        case TOK_SEQ:
        case TOK_LOGIC:
            if(ast->nodes.connector.op){
                printf("%s\n", ast->nodes.connector.op);
            }

            if(ast->nodes.connector.left){
                printf("[L]");
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
            printf("Unknown command type\n");
            break;
    }
}





