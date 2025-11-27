#include "parser.h"

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

bool is_connector(char* type){
    if(type == NULL) return false;
    if(strcmp(type, "||")==0 || strcmp(type, "&&")==0 || strcmp(type, "|")==0 || strcmp(type, ";")==0){
        return true;
    }
    return false;
}

bool is_redirect(char* type){
    if(type == NULL) return false;
    if(strcmp(type, "<")==0 || strcmp(type, ">")==0 || strcmp(type, ">>")==0){
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
