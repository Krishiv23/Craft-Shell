#ifndef PROMPT_H
#define PROMPT_H

#include "headers.h"

//prompt style
#define CYAN "\e[1;36m"
#define WHITE "\e[0;97m"
#define BOLD "\e[1m"
#define RESET "\e[0m"

void prompt();
char* read_input();

#endif //PROMPT_H