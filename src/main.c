#include "headers.h"
#include "buildins.h"
#include "prompt.h"
#include "parser.h"
#include "lexer.h"
#include "execution_engine.h"

int token_count;
int current_token;
int if_exit;

void ascii();

int main(int UNUSED argc, char** UNUSED argv){
    ascii();
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

    if(if_exit==1){
        if_exit=0;
        exit(exit_stat);
    }
    }
    return 0;
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

void ascii(){
    FILE* banner = fopen("ascii.txt", "r");
    if(!banner) return;
    char line[1024];

    printf("\n\n");

    while(fgets(line, sizeof(line), banner)){
        fputs(line, stdout);
    }

    printf("\n\n");

    fclose(banner);
}
