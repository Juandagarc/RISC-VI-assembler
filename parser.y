%{
#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.h"
#include "decoder.h"

/* Prototype function for lexycal analysis */
int yylex();
void yyerror(const char *s);
extern FILE* yyin;
%}

%union {
    int   iValue; /* numeric value (immediate) */
    char* sValue; /* strings (instructions, registers, tags) */
};
//token for intructions
%token <sValue> R_T_INSTRUCTION
%token <sValue> I_T_INSTRUCTION
%token <sValue> S_T_INSTRUCTION
%token <sValue> B_T_INSTRUCTION
%token <sValue> U_T_INSTRUCTION
%token <sValue> T_REGISTER
%token <sValue> T_USETAG
%token <sValue> T_IMMEDIATE

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
    R_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_REGISTER
    {
        printf("Type R \n" );
        add_symb_tab($1, R_INSTRUCTION,  $2, $4, $6, 0);
        printf("Instruction: %s %s, %s, %s\n", $1, $2, $4, $6);
    }
    | I_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_IMMEDIATE
        {
            printf("Type I \n" );
            /* store immediate as symbol with its numeric value */
            long imm = strtol($6, NULL, 0);
            char imm_buf[32];
            snprintf(imm_buf, sizeof imm_buf, "%ld", imm);
            add_symb_tab($1, I_INSTRUCTION, $2, $4, "", (int)imm);
            printf("Instruction: %s %s, %s, %ld\n", $1, $2, $4, imm);
        }
    | S_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_IMMEDIATE
        {
            printf("Type S \n" );
            
            long imm = strtol($6, NULL, 0);
            char imm_buf[32];
            snprintf(imm_buf, sizeof imm_buf, "%ld", imm);
            add_symb_tab($1, S_INSTRUCTION, "", $2, $4, (int)imm);
            printf("Instruction: %s %s, %s, %ld\n", $1, $2, $4, imm);
        }
    | B_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_USETAG
        {
            printf("Type B (label) \n" );
            add_symb_tab($1, B_INSTRUCTION, "", $2, $4, 0);
            add_symb_tab($6, LABEL, "", "", "", 0);
            printf("Instruction: %s %s, %s, %s\n", $1, $2, $4, $6);
        }
    | B_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_IMMEDIATE
        {
            printf("Type B (imm) \n" );
            long imm = strtol($6, NULL, 0);
            char imm_buf[32];
            snprintf(imm_buf, sizeof imm_buf, "%ld", imm);
            add_symb_tab($1, B_INSTRUCTION, "", $2, $4, (int)imm);
            printf("Instruction: %s %s, %s, %ld\n", $1, $2, $4, imm);
        }
    | U_T_INSTRUCTION T_REGISTER T_COMMA T_IMMEDIATE
        {
            printf("Type U \n" );
            long imm = strtol($4, NULL, 0);
            char imm_buf[32];
            snprintf(imm_buf, sizeof imm_buf, "%ld", imm);
            add_symb_tab($1, U_INSTRUCTION, $2, "", "", (int)imm);
            printf("Instruction: %s %s, %ld\n", $1, $2, imm);
        }
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
    cleanup_symbol_table();
    fclose(f);
    return 0;
}