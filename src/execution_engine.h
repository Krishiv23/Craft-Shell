#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include "headers.h"
#include "parser.h"
#include "buildins.h"

int execute_ast(AST_node* ast);
int exec_cmd(int size, char*** cmd_arr, AST_node* redir);
int exec_pipe(AST_node* left, AST_node* right);
int exec_connetor(AST_node* node);

#endif //EXECUTION_ENGINE_H