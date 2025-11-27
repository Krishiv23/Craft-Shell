#ifndef LEXER_H
#define LEXER_H

#include "headers.h"

int rm_space(char* input);
void get_token(char* input, char*** tokens);

#endif //LEXER_H