#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"

#define MAX_SIZE 3000

typedef struct Symbol_T {
    char* name;          
    Symbol_type type;
    const char* opcode;
    char* rd;            
    char* rs1;
    char* rs2;
    char* imm;
    char* funct3;
    char* funct7;
} Symbol_T;

static Symbol_T symbols_table[MAX_SIZE];
static int symbol_count = 0;

extern void yyerror(const char* s);

/****************************************************/
/*             SYMBOL TABLE Functions               */
/****************************************************/

int check_symbol_table_full(){
    return symbol_count >= MAX_SIZE;
}

int is_duplicate(const char *name){
    if (name == NULL) return 0;
    
    for (int i = 0; i < symbol_count; i++){
        if (symbols_table[i].name && strcmp(symbols_table[i].name, name) == 0){
            return 1;
        }
    }
    return 0;
}

// Auxiliary function to safely duplicate strings
char* safe_strdup(const char* str) {
    if (str == NULL) {
        char* empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    
    size_t len = strlen(str);
    if (len == 0) {
        char* empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    
    char* copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

int add_symb_tab(const char *name, Symbol_type type, char *rd, char *rs1, char *rs2, int value){
    if (check_symbol_table_full()){
        yyerror("Out of memory\n");
        return -1;
    }
    
    if (is_duplicate(name)) {
        printf("Warning: Duplicate symbol %s\n", name);
        // Decidir si permitir duplicados o no
    }

    // Crear nuevo símbolo con memoria dinámica
    symbols_table[symbol_count].name = safe_strdup(name);
    symbols_table[symbol_count].type = type;
    if (symbols_table[symbol_count].type == LABEL) {
        symbols_table[symbol_count].opcode = "";
    } else {
        symbols_table[symbol_count].opcode = instruction_opcode(name, type);
    }
    
    // Usar las funciones mejoradas que retornan memoria dinámica
    symbols_table[symbol_count].rd  = register_to_binary(rd);
    symbols_table[symbol_count].rs1 = register_to_binary(rs1);
    symbols_table[symbol_count].rs2 = register_to_binary(rs2);
    symbols_table[symbol_count].imm = immediate_to_binary(value, type, name);
    symbols_table[symbol_count].funct3 = funct3_binary(name);
    symbols_table[symbol_count].funct7 = funct7_binary(name);

    return symbol_count++;
}

int symbol_count_func(){
    return symbol_count;
}

void print_table(){
    static const char *types[] = {"LABEL", "R_INSTRUCTION", "I_INSTRUCTION", 
                                  "S_INSTRUCTION", "B_INSTRUCTION", "U_INSTRUCTION"};
    printf("\n=== SYMBOL TABLE ===\n");
    printf("LINE\tNAME\t\tTYPE\t\tOPCODE\t\tRD\tRS1\tRS2\tFUNCT3\tFUNCT7\tIMM\n");
    printf("-------------------------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < symbol_count; i++) {
        printf("%-5d\t%-10s\t%-12s\t%-8s\t%-4s\t%-4s\t%-4s\t%-4s\t%-4s\t%-8s\n",
            i,
            symbols_table[i].name ? symbols_table[i].name : "NULL",
            types[symbols_table[i].type],
            symbols_table[i].opcode ? symbols_table[i].opcode : "NULL",
            symbols_table[i].rd ? symbols_table[i].rd : "---",
            symbols_table[i].rs1 ? symbols_table[i].rs1 : "---",
            symbols_table[i].rs2 ? symbols_table[i].rs2 : "---",
            symbols_table[i].funct3 ? symbols_table[i].funct3 : "---",
            symbols_table[i].funct7 ? symbols_table[i].funct7 : "---",
            symbols_table[i].imm ? symbols_table[i].imm : "---"
        );
    }
    printf("-------------------------------------------------------------------------------------------------------------------\n\n");
}

void cleanup_symbol_table() {
    for (int i = 0; i < symbol_count; i++) {
        free(symbols_table[i].name);
        free(symbols_table[i].rd);
        free(symbols_table[i].rs1);
        free(symbols_table[i].rs2);
        free(symbols_table[i].imm);
        free(symbols_table[i].funct3);
        free(symbols_table[i].funct7);
    }
    symbol_count = 0;
}

#endif