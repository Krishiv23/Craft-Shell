#include "lexer.h"

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