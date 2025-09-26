#ifndef SYMBOL_TABLE
#define SYMBOL_TABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "decoder.h"

#define MAX_SIZE 3000
#define MAX_LABELS 1000

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
    int address;  /* Address where this instruction is located */
    int value;      // <-- Añade esta línea para el valor del dato
    int data_size;  // <-- Añade esta línea (1 para byte, 4 para word)
} Symbol_T;

typedef struct Label_T {
    char* name;
    int address;
} Label_T;

static Symbol_T symbols_table[MAX_SIZE];
static Label_T labels_table[MAX_LABELS];
static int symbol_count = 0;
static int label_count = 0;

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

    // Crear nuevo símbolo con memoria dinámica
    symbols_table[symbol_count].name = safe_strdup(name);
    symbols_table[symbol_count].type = type;
    symbols_table[symbol_count].address = symbol_count * 4; /* Simple addressing */

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
    symbols_table[symbol_count].funct3 = safe_strdup(funct3_binary(name));
    symbols_table[symbol_count].funct7 = safe_strdup(funct7_binary(name));
    symbols_table[symbol_count].value = value;  // <-- Almacena el valor
    symbols_table[symbol_count].data_size = 4;  // <-- Tamaño por defecto en bytes (puedes cambiarlo según sea necesario)

    return symbol_count++;
}

// Coloca esto después de la función add_symb_tab
int add_data_symb(const char *name, int value, int size, int address) {
    if (check_symbol_table_full()) {
        yyerror("Out of memory for data\n");
        return -1;
    }

    int index = symbol_count;
    symbols_table[index].name = safe_strdup(name);
    symbols_table[index].type = DATA;
    symbols_table[index].address = address;
    symbols_table[index].value = value;
    symbols_table[index].data_size = size;

    // Importante: Pon a NULL los punteros que no se usan para evitar errores de liberación de memoria
    symbols_table[index].opcode = NULL;
    symbols_table[index].rd  = NULL;
    symbols_table[index].rs1 = NULL;
    symbols_table[index].rs2 = NULL;
    symbols_table[index].imm = NULL;
    symbols_table[index].funct3 = NULL;
    symbols_table[index].funct7 = NULL;

    return symbol_count++;
}

/****************************************************/
/*             LABEL TABLE Functions                */
/****************************************************/

int add_label_to_table(const char* name, int address) {
    if (label_count >= MAX_LABELS) {
        yyerror("Too many labels");
        return -1;
    }

    /* Check for duplicate labels */
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels_table[i].name, name) == 0) {
            yyerror("Duplicate label definition");
            return -1;
        }
    }

    labels_table[label_count].name = safe_strdup(name);
    labels_table[label_count].address = address;
    label_count++;

    return 0;
}

int get_label_address(const char* name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels_table[i].name, name) == 0) {
            return labels_table[i].address;
        }
    }
    return -1; /* Label not found */
}

/****************************************************/
/*             MACHINE CODE GENERATION              */
/****************************************************/

uint32_t generate_machine_code(int symbol_index) {
    if (symbol_index >= symbol_count) return 0;

    Symbol_T* sym = &symbols_table[symbol_index];

    // Añade este bloque al inicio de la función
    if (sym->type == DATA) {
        // Para .word y .byte, el "código máquina" es simplemente el valor en sí.
        // La lógica de empaquetado de bytes se manejará en la generación de archivos.
        return (uint32_t)sym->value;
    }

    uint32_t machine_code = 0;

    /* Convert binary strings to integers */
    uint32_t opcode = 0, rd = 0, rs1 = 0, rs2 = 0, funct3 = 0, funct7 = 0;
    int32_t imm = 0;

    if (sym->opcode) opcode = strtol(sym->opcode, NULL, 2);
    if (sym->rd) rd = strtol(sym->rd, NULL, 2);
    if (sym->rs1) rs1 = strtol(sym->rs1, NULL, 2);
    if (sym->rs2) rs2 = strtol(sym->rs2, NULL, 2);
    if (sym->funct3) funct3 = strtol(sym->funct3, NULL, 2);
    if (sym->funct7) funct7 = strtol(sym->funct7, NULL, 2);
    if (sym->imm) imm = strtol(sym->imm, NULL, 2);

    switch (sym->type) {
        case R_INSTRUCTION:
            machine_code = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) |
                          (funct3 << 12) | (rd << 7) | opcode;
            break;

        case I_INSTRUCTION:
            machine_code = (imm << 20) | (rs1 << 15) | (funct3 << 12) |
                          (rd << 7) | opcode;
            break;

        case S_INSTRUCTION:
            machine_code = (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) |
                          (funct3 << 12) | ((imm & 0x1F) << 7) | opcode;
            break;

        case B_INSTRUCTION:
            machine_code = (((imm >> 12) & 0x1) << 31) | (((imm >> 5)  & 0x3F) << 25) |
                          (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
                          (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 0x1) << 7) | opcode;
            break;

        case U_INSTRUCTION:
            machine_code = (imm << 12) | (rd << 7) | opcode;
            break;

        case J_INSTRUCTION:
            machine_code = ((imm & 0x100000) << 11) | ((imm & 0x7FE) << 20) |
                          ((imm & 0x800) << 9) | ((imm & 0xFF000)) |
                          (rd << 7) | opcode;
            break;

        default:
            machine_code = 0;
    }

    return machine_code;
}

int symbol_count_func(){
    return symbol_count;
}

void print_table(){
    printf("\n=== LABEL TABLE ===\n");
    printf("%-15s %-10s\n", "Label", "Address");
    printf("--------------------------\n");
    for(int i = 0; i < label_count; i++){
        printf("%-15s 0x%08x\n", labels_table[i].name, labels_table[i].address);
    }

    printf("\n");

    printf("\n=== SYMBOL TABLE ===\n");
    printf("%-15s %-10s %-10s %-8s %-8s %-8s %-8s %-8s %-12s\n",
           "Name", "Type", "Opcode", "rd", "rs1", "rs2", "funct3", "funct7", "Immediate");
    printf("----------------------------------------------------------------------------------------------------------\n");

    // Dentro del bucle for de print_table
    for(int i = 0; i < symbol_count; i++){
        if (symbols_table[i].type == DATA) {
            printf("%-15s %-10d %-10s %-8s %-8s %-8s %-8s %-8s %-12d (size: %d)\n",
                   symbols_table[i].name,
                   symbols_table[i].type,
                   "---", "---", "---", "---", "---", "---",
                   symbols_table[i].value, symbols_table[i].data_size);
        } else {
            printf("%-15s %-10d %-10s %-8s %-8s %-8s %-8s %-8s %-12s\n",
                   symbols_table[i].name ? symbols_table[i].name : "---",
                   symbols_table[i].type,
                   symbols_table[i].opcode ? symbols_table[i].opcode : "---",
                   symbols_table[i].rd ? symbols_table[i].rd : "---",
                   symbols_table[i].rs1 ? symbols_table[i].rs1 : "---",
                   symbols_table[i].rs2 ? symbols_table[i].rs2 : "---",
                   symbols_table[i].funct3 ? symbols_table[i].funct3 : "---",
                   symbols_table[i].funct7 ? symbols_table[i].funct7 : "---",
                   symbols_table[i].imm ? symbols_table[i].imm : "---");
        }
    }

    printf("\n");
}

void cleanup_symbol_table(){
    for(int i = 0; i < symbol_count; i++){
        free(symbols_table[i].name);
        free(symbols_table[i].rd);
        free(symbols_table[i].rs1);
        free(symbols_table[i].rs2);
        free(symbols_table[i].imm);
        free(symbols_table[i].funct3);
        free(symbols_table[i].funct7);
    }

    for(int i = 0; i < label_count; i++){
        free(labels_table[i].name);
    }

    symbol_count = 0;
    label_count = 0;
}

#endif

