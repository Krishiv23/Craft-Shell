#ifndef PARSER_H
#define PARSER_H

#include "headers.h"

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

void create_node_command(AST_node** node, char* cmd);
void create_node_argument(AST_node** node, char* arg);
AST_node* create_node_connector(char** operator, int type, AST_node* left, AST_node* right);
void create_node_redirect(AST_node** node, char* op, char* destination);
bool is_connector(char* type);
bool is_redirect(char* type);
AST_node* parse_redirect(char*** tokens);
AST_node* parse_cmd_line(char*** tokens);
AST_node* parse_connector(AST_node* left, char*** tokens);
AST_node* parse_sim_cmd(char*** tokens);
AST_node* parse_cmd(char*** tokens);
AST_node* parse_arg(char*** tokens);
void free_ast(AST_node* ast);

#endif //PARSER_H