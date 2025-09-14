%{
#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.h"

/* Prototype function for lexycal analysis */
int yylex();
void yyerror(const char *s);
extern FILE* yyin;
%}

%union {
    char* iValue; /* integer value */
    char sIndex; /* symbol table index */
};
//token for intructions
%token <iValue> T_INSTRUCTION
%token <iValue> T_REGISTER
%token <iValue> T_USETAG

%token T_COMMA
%token T_EOL //End of line
%%


program:
    /* empty */
  | program line
  ;
line:
      instruction T_EOL
    | T_EOL             /* Allow white lines */
    ;

instruction:
    T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER
    {
        add_symb_tab($1, INSTRUCTION, 0);
        add_symb_tab($2, REGISTER, 0);
        add_symb_tab($4, REGISTER, 0);
        printf("Instruction: %s %s, %s\n", $1, $2, $4);
    }
    | T_INSTRUCTION T_REGISTER T_COMMA T_USETAG
    {
        add_symb_tab($1, INSTRUCTION, 0);
        add_symb_tab($2, REGISTER, 0);
        add_symb_tab($4, LABEL, 0);
        printf("Instruction: %s %s, %s\n", $1, $2, $4);
    }
    | T_INSTRUCTION T_REGISTER
    {
        add_symb_tab($1, INSTRUCTION, 0);
        add_symb_tab($2, REGISTER, 0);
        printf("Instruction: %s %s\n", $1, $2);
    };
%%


void yyerror(const char *s) {
    fprintf(stderr, "Error de sintaxis: %s\n", s);
}

int main(void) {
    FILE* f = fopen("test.asm", "r");
    if (!f) {
        perror("test.asm");
        return 1;
    }
    yyin = f;
    printf("Iniciando análisis sintáctico...\n");
    yyparse();
    printf("Análisis finalizado.\n");
    print_table();
    fclose(f);
    return 0;
}