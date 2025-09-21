#ifndef DECODER
#define DECODER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    LABEL,
    R_INSTRUCTION,
    I_INSTRUCTION,
    S_INSTRUCTION,
    B_INSTRUCTION,
    U_INSTRUCTION,
} Symbol_type;

/****************************************************/
/*           DECODER / ENCODER Functions            */
/****************************************************/

const char* instruction_opcode(const char *name, Symbol_type type){
    // Arrays of opcodes for different instruction types
    static const char* opcodes[] = {
        "0110011", "0100011", "1100011" 
    };
    static const char* I_type_opcodes[] = {
        "0000011", "0010011", "1100111"
    };

    if (type != I_INSTRUCTION && type != U_INSTRUCTION){
        printf("Extracting opcode for instruction %s type: %d\n", name, type);
        if (type == S_INSTRUCTION || type == B_INSTRUCTION) {
            return opcodes[type - 2];
        }
        return opcodes[type - 1];
    }
    
    if (type == I_INSTRUCTION){
        if (strcmp(name, "jalr") == 0){
            return I_type_opcodes[2];
        } else if ((strcmp(name, "lw") == 0) || (strcmp(name, "lh") == 0) || 
                   (strcmp(name, "lb") == 0) || (strcmp(name, "lhu") == 0) || 
                   (strcmp(name, "lbu") == 0)){
            return I_type_opcodes[0];
        } else {
            return I_type_opcodes[1];
        }
    }
    
    if (type == U_INSTRUCTION){
        if (strcmp(name, "lui") == 0){
            return "0110111";
        } else {
            return "0010111";
        }
    }
    
    return "0000000"; // Error
}

int value_register(const char *name) {
    static const char *reg_name[] = {
        "zero", "ra", "sp", "gp", "tp", 
        "t0", "t1", "t2", "s0", "s1",
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
        "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",
        "s10", "s11", "t3", "t4", "t5", "t6"
    };
    
    if (name == NULL) return -1;
    
    if (name[0] != 'x') {
        for (int i = 0; i < 32; i++) {
            if (strcmp(name, reg_name[i]) == 0) {
                return i;
            }
        }
    } else {
        int number = atoi(name + 1);
        if (number >= 0 && number < 32) {
            return number;
        }
    }
    return -1;
}

char* reg_to_binary(int reg_num) {
    if (reg_num < 0 || reg_num > 31) {
        return NULL;
    }
    
    // Use malloc for dynamic memory allocation
    char* binary = malloc(6);
    if (!binary) return NULL;
    
    binary[5] = '\0';
    
    for (int i = 4; i >= 0; i--) {
        binary[i] = (reg_num & 1) + '0';
        reg_num >>= 1;
    }
    
    return binary;
}

char* register_to_binary(const char* reg_name) {
    if (reg_name == NULL || strlen(reg_name) == 0) {
        char* empty = malloc(4);
        if (empty) strcpy(empty, "---");
        return empty;
    }
    
    int reg_value = value_register(reg_name);
    if (reg_value == -1) {
        char* empty = malloc(4);
        if (empty) strcpy(empty, "---");
        return empty;
    }
    
    return reg_to_binary(reg_value);
}

int is_immediate_valid(int value, Symbol_type type) {
    switch(type) {
        case I_INSTRUCTION:
        case S_INSTRUCTION:
            return (value >= -2048 && value <= 2047);
            
        case B_INSTRUCTION:
            return (value >= -4096 && value <= 4095) && 
                   (value % 2 == 0);
            
        case U_INSTRUCTION:
            return (value >= 0 && value <= 1048575);
            
        default:
            return 0;
    }
}

char* immediate_to_binary(int value, Symbol_type type) {
    if (!is_immediate_valid(value, type)) {
        printf("Warning: Invalid immediate value %d for type %d\n", value, type);
        char* empty = malloc(4);
        if (empty) strcpy(empty, "---");
        return empty;
    }
    
    int bits = (type == U_INSTRUCTION) ? 20 : 
               (type == B_INSTRUCTION) ? 13 : 12;
    
    char* binary = malloc(bits + 1);
    if (!binary) {
        char* empty = malloc(4);
        if (empty) strcpy(empty, "---");
        return empty;
    }
    
    memset(binary, '0', bits);
    binary[bits] = '\0';
    
    // 2 complement representation
    if (value < 0) {
        unsigned int mask = (1U << bits) - 1;
        unsigned int uvalue = ((unsigned int)value) & mask;
        
        for (int i = bits - 1; i >= 0; i--) {
            binary[i] = (uvalue & 1) + '0';
            uvalue >>= 1;
        }
    } else {
        unsigned int uvalue = (unsigned int)value;
        for (int i = bits - 1; i >= 0; i--) {
            binary[i] = (uvalue & 1) + '0';
            uvalue >>= 1;
        }
    }
    
    return binary;
}

#endif