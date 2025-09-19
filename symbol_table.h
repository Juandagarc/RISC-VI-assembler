#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 3000

typedef enum { // Create a enumarate for most easiest variable declarations
    LABEL,
    R_INSTRUCTION,
    I_INSTRUCTION,
    S_INSTRUCTION,
    B_INSTRUCTION,
    U_INSTRUCTION,
    REGISTER,
    IMMEDIATE
} Symbol_type;

typedef struct Symbol_T { // Create the structure for the hash table
    char name[32];
    Symbol_type type;
    int value;
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
    for (int i = 0; i < symbol_count; i++){
        if (strcmp(symbols_table[i].name, name) == 0){
            return 1;
        }
    }
    return 0;
}

int add_symb_tab(const char *name, Symbol_type type, int value){
    if (check_symbol_table_full()){ // Check that the user try to add more than the capacity of the table
        yyerror("Out of memory\n");
        return -1;
    };
    is_duplicate(name); // Check if the symbol already exists

    // Create new symbol
    strncpy(symbols_table[symbol_count].name, name, 31);
    symbols_table[symbol_count].name[31] = '\0';
    symbols_table[symbol_count].type = type;
    symbols_table[symbol_count].value = value;
    return symbol_count++;
};

int find_symbols(const char *name){
    for (int i = 0; i < symbol_count; i++){
        if (strcmp(symbols_table[i].name, name) == 0){
            return i;
        }
    }
    return -1;
};

void print_table(){
    char *types[] = {"LABEL", "R_INSTRUCTION", "I_INSTRUCTION", "S_INSTRUCTION", "B_INSTRUCTION", "U_INSTRUCTION", "REGISTER", "IMMEDIATE"};
    printf("\n=== SYMBOL TABLE ===\n");
    printf("NAME\t\tTYPE\t\tVALUE\n");
    printf("----------------------------------\n");
    for (int i = 0; i < symbol_count; i++) {
        printf("%-10s\t%-12s\t%d\n",
            symbols_table[i].name,
            types[symbols_table[i].type],
            symbols_table[i].value);
    }
    printf("----------------------------------\n\n");
}

#endif
