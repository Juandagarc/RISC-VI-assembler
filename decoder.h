#ifndef DECODER
#define DECODER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef enum {
    LABEL,
    R_INSTRUCTION,
    I_INSTRUCTION,
    S_INSTRUCTION,
    B_INSTRUCTION,
    U_INSTRUCTION,
    J_INSTRUCTION,
    DATA // <-- Añade esta línea
} Symbol_type;

/****************************************************/
/*           DECODER / ENCODER Functions            */
/****************************************************/

const char* instruction_opcode(const char *name, Symbol_type type){
    // Arrays of opcodes for different instruction types
    static const char* R_opcodes = "0110011";
    static const char* S_opcodes = "0100011";
    static const char* B_opcodes = "1100011";
    static const char* J_opcodes = "1101111";

    static const char* I_type_opcodes[] = {
        "0000011", "0010011", "1100111"
    };

    switch(type) {
        case R_INSTRUCTION:
            return R_opcodes;
        case S_INSTRUCTION:
            return S_opcodes;
        case B_INSTRUCTION:
            return B_opcodes;
        case J_INSTRUCTION:
            return J_opcodes;
        case I_INSTRUCTION:
            if (strcmp(name, "jalr") == 0){
                return I_type_opcodes[2];
            } else if ((strcmp(name, "lw") == 0) || (strcmp(name, "lh") == 0) ||
                       (strcmp(name, "lb") == 0) || (strcmp(name, "lhu") == 0) ||
                       (strcmp(name, "lbu") == 0)){
                return I_type_opcodes[0];
            } else {
                return I_type_opcodes[1];
            }
        case U_INSTRUCTION:
            if (strcmp(name, "lui") == 0){
                return "0110111";
            } else {
                return "0010111";
            }
        default:
            return "0000000"; // Error
    }
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
    if (strcmp(name, "fp") == 0) return 8; /* alias de s0 */
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

// Validation functions
int validate_immediate_range(int value, Symbol_type type, const char* instruction) {
    switch(type) {
        case R_INSTRUCTION:
            return 1; /* No inmediato, se ignora */
        case I_INSTRUCTION:
            if (strcmp(instruction, "slli") == 0 || strcmp(instruction, "srli") == 0 ||
                strcmp(instruction, "srai") == 0) {
                return (value >= 0 && value <= 31); // Shift amount
            }
            return (value >= -2048 && value <= 2047);
        case S_INSTRUCTION:
            return (value >= -2048 && value <= 2047);
        case B_INSTRUCTION:
            return (value >= -4096 && value <= 4094 && (value % 2 == 0));
        case U_INSTRUCTION:
            return (value >= 0 && value <= 1048575); // 20 bits
        case J_INSTRUCTION:
            return (value >= -1048576 && value <= 1048574 && (value % 2 == 0));
        default:
            return 0;
    }
}

int validate_register(const char* reg_name) {
    return value_register(reg_name) != -1;
}

char* reg_to_binary(int reg_num) {
    if (reg_num < 0 || reg_num > 31) {
        return NULL;
    }
    
    char* binary = malloc(6);
    if (!binary) return NULL;
    
    binary[5] = '\0';
    
    for (int i = 4; i >= 0; i--) {
        binary[i] = (reg_num & 1) + '0';
        reg_num >>= 1;
    }
    
    return binary;
}

char* register_to_binary(const char *name) {
    if (name == NULL || strlen(name) == 0) {
        return NULL;
    }
    
    int reg_num = value_register(name);
    return reg_to_binary(reg_num);
}

char* immediate_to_binary(int value, Symbol_type type, const char* instruction) {
    if (type == R_INSTRUCTION) {
        return NULL; /* No generar campo ni warning */
    }
    if (!validate_immediate_range(value, type, instruction)) {
        printf("Warning: Invalid immediate value %d for type %d\n", value, type);
        return NULL;
    }

    // Handle different immediate sizes based on instruction type
    int bits = 12; // Default for I-type
    uint32_t mask = 0xFFF;

    switch(type) {
        case U_INSTRUCTION:
            bits = 20;
            mask = 0xFFFFF; /* Se asume value ya viene en 20 bits superiores */
            break;
        case J_INSTRUCTION:
            bits = 20;
            mask = 0xFFFFF;
            break;
        case B_INSTRUCTION:
            bits = 13;
            mask = 0x1FFF;
            break;
        case S_INSTRUCTION:
        case I_INSTRUCTION:
            if (strcmp(instruction, "slli") == 0 || strcmp(instruction, "srli") == 0 ||
                strcmp(instruction, "srai") == 0) {
                bits = 5; // Shift amount
                mask = 0x1F;
            } else {
                bits = 12;
                mask = 0xFFF;
            }
            break;
        default:
            bits = 12;
            mask = 0xFFF;
            break;
    }

    char* binary = malloc(bits + 1);
    if (!binary) return NULL;

    binary[bits] = '\0';

    // Convert to binary
    uint32_t unsigned_value = value & mask;
    for (int i = bits - 1; i >= 0; i--) {
        binary[bits - 1 - i] = ((unsigned_value >> i) & 1) + '0';
    }

    return binary;
}

const char* funct3_binary(const char* instruction) {
    if (!instruction) return "000";

    // R-type instructions
    if (strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0) return "000";
    if (strcmp(instruction, "sll") == 0) return "001";
    if (strcmp(instruction, "slt") == 0) return "010";
    if (strcmp(instruction, "sltu") == 0) return "011";
    if (strcmp(instruction, "xor") == 0) return "100";
    if (strcmp(instruction, "srl") == 0 || strcmp(instruction, "sra") == 0) return "101";
    if (strcmp(instruction, "or") == 0) return "110";
    if (strcmp(instruction, "and") == 0) return "111";

    // I-type instructions
    if (strcmp(instruction, "addi") == 0) return "000";
    if (strcmp(instruction, "slti") == 0) return "010";
    if (strcmp(instruction, "sltiu") == 0) return "011";
    if (strcmp(instruction, "xori") == 0) return "100";
    if (strcmp(instruction, "ori") == 0) return "110";
    if (strcmp(instruction, "andi") == 0) return "111";
    if (strcmp(instruction, "slli") == 0) return "001";
    if (strcmp(instruction, "srli") == 0 || strcmp(instruction, "srai") == 0) return "101";

    // Load instructions
    if (strcmp(instruction, "lb") == 0) return "000";
    if (strcmp(instruction, "lh") == 0) return "001";
    if (strcmp(instruction, "lw") == 0) return "010";
    if (strcmp(instruction, "lbu") == 0) return "100";
    if (strcmp(instruction, "lhu") == 0) return "101";

    // Store instructions
    if (strcmp(instruction, "sb") == 0) return "000";
    if (strcmp(instruction, "sh") == 0) return "001";
    if (strcmp(instruction, "sw") == 0) return "010";

    // Branch instructions
    if (strcmp(instruction, "beq") == 0) return "000";
    if (strcmp(instruction, "bne") == 0) return "001";
    if (strcmp(instruction, "blt") == 0) return "100";
    if (strcmp(instruction, "bge") == 0) return "101";
    if (strcmp(instruction, "bltu") == 0) return "110";
    if (strcmp(instruction, "bgeu") == 0) return "111";

    // JALR
    if (strcmp(instruction, "jalr") == 0) return "000";

    return "000"; // Default
}

const char* funct7_binary(const char* instruction) {
    if (!instruction) return "0000000";

    // Only R-type and some I-type (shift) instructions use funct7
    if (strcmp(instruction, "sub") == 0 || strcmp(instruction, "sra") == 0 ||
        strcmp(instruction, "srai") == 0) {
        return "0100000";
    }

    return "0000000"; // Default for most instructions
}

void print_error(const char* message, const char* instruction, int line_number) {
    fprintf(stderr, "Error at line %d in instruction '%s': %s\n",
            line_number, instruction ? instruction : "unknown", message);
}

#endif
