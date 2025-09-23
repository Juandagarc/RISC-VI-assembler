%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symbol_table.h"
#include "decoder.h"

/* Prototype function for lexical analysis */
int yylex();
void yyerror(const char *s);
extern FILE* yyin;

/* Global variables for two-pass assembler */
int current_address = 0;
int pass_number = 1;
char* input_filename = NULL;
char* hex_filename = NULL;
char* bin_filename = NULL;

/* Function prototypes */
void expand_pseudoinstruction(const char* pseudo, char* rd, char* rs1, char* rs2, char* imm);
void generate_output_files();
void reset_for_second_pass();
%}

%union {
    int   iValue; /* numeric value (immediate) */
    char* sValue; /* strings (instructions, registers, tags) */
};

/* Tokens for instructions */
%token <sValue> R_T_INSTRUCTION
%token <sValue> I_T_INSTRUCTION
%token <sValue> S_T_INSTRUCTION
%token <sValue> B_T_INSTRUCTION
%token <sValue> U_T_INSTRUCTION
%token <sValue> J_T_INSTRUCTION
%token <sValue> PSEUDO_INSTRUCTION
%token <sValue> T_REGISTER
%token <sValue> T_USETAG
%token <sValue> T_IMMEDIATE
%token <sValue> T_LABEL

/* Other tokens */
%token T_COMMA
%token T_LPAREN
%token T_RPAREN
%token T_EOL
%token T_TEXT
%token T_DATA

%%


program:
    /* empty */
  | program line
  ;
line:
    directive T_EOL
  | label T_EOL
  | instruction T_EOL
  | T_EOL             /* Allow empty lines */
  ;

directive:
    T_TEXT {
            current_address = 0;
            if (pass_number == 1) {
                printf("Text segment starts at address 0x%08x\n", current_address);
            }
    }
  | T_DATA {
        if (pass_number == 1) {
            printf("Data segment directive found\n");
        }
    }
  ;

label:
    T_LABEL {
        if (pass_number == 1) {
            add_label_to_table($1, current_address);
            printf("Label: %s at address 0x%08x\n", $1, current_address);
        }
    }
  ;

instruction:
    /* R-type instructions */
    R_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_REGISTER {
        if (pass_number == 2) {
            add_symb_tab($1, R_INSTRUCTION, $2, $4, $6, 0);
            printf("R-type: %s %s, %s, %s\n", $1, $2, $4, $6);
        }
        current_address += 4;
    }

    /* I-type arithmetic/logic/shifts/jalr: rd, rs1, imm */
  | I_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_IMMEDIATE {
        if (pass_number == 2) {
            long imm = strtol($6, NULL, 0);
            add_symb_tab($1, I_INSTRUCTION, $2, $4, "", (int)imm);
            printf("I-type: %s %s, %s, %ld\n", $1, $2, $4, imm);
        }
        current_address += 4;
    }

    /* I-type loads: rd, imm(rs1) */
  | I_T_INSTRUCTION T_REGISTER T_COMMA T_IMMEDIATE T_LPAREN T_REGISTER T_RPAREN {
        if (pass_number == 2) {
            long imm = strtol($4, NULL, 0);
            add_symb_tab($1, I_INSTRUCTION, $2, $6, "", (int)imm);
            printf("I-load: %s %s, %ld(%s)\n", $1, $2, imm, $6);
        }
        current_address += 4;
    }

    /* S-type stores: rs2, imm(rs1) */
  | S_T_INSTRUCTION T_REGISTER T_COMMA T_IMMEDIATE T_LPAREN T_REGISTER T_RPAREN {
        if (pass_number == 2) {
            long imm = strtol($4, NULL, 0);
            add_symb_tab($1, S_INSTRUCTION, "", $6, $2, (int)imm);
            printf("S-type: %s %s, %ld(%s)\n", $1, $2, imm, $6);
        }
        current_address += 4;
    }

    /* B-type with label */
  | B_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($6);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            add_symb_tab($1, B_INSTRUCTION, "", $2, $4, offset);
            printf("B-type: %s %s, %s, %s (offset: %d)\n", $1, $2, $4, $6, offset);
        }
        current_address += 4;
    }

    /* B-type with immediate */
  | B_T_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_IMMEDIATE {
        if (pass_number == 2) {
            long imm = strtol($6, NULL, 0);
            add_symb_tab($1, B_INSTRUCTION, "", $2, $4, (int)imm);
            printf("B-type imm: %s %s, %s, %ld\n", $1, $2, $4, imm);
        }
        current_address += 4;
    }

    /* U-type instructions */
  | U_T_INSTRUCTION T_REGISTER T_COMMA T_IMMEDIATE {
        if (pass_number == 2) {
            long imm = strtol($4, NULL, 0);
            add_symb_tab($1, U_INSTRUCTION, $2, "", "", (int)imm);
            printf("U-type: %s %s, %ld\n", $1, $2, imm);
        }
        current_address += 4;
    }

    /* J-type: jal rd, label */
  | J_T_INSTRUCTION T_REGISTER T_COMMA T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($4);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            add_symb_tab($1, J_INSTRUCTION, $2, "", "", offset);
            printf("J-type: %s %s, %s (offset: %d)\n", $1, $2, $4, offset);
        }
        current_address += 4;
    }

    /* J-type: jal label (defaults rd=x1) */
  | J_T_INSTRUCTION T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($2);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            add_symb_tab("jal", J_INSTRUCTION, "x1", "", "", offset);
            printf("J-type: jal x1, %s (offset: %d)\n", $2, offset);
        }
        current_address += 4;
    }

    /* Pseudoinstructions - no args (nop) */
  | PSEUDO_INSTRUCTION {
        if (pass_number == 2) {
            expand_pseudoinstruction($1, "", "", "", "");
        }
        current_address += 4;
    }

    /* Pseudoinstructions - one register (jr, ret-like when tokenized accordingly) */
  | PSEUDO_INSTRUCTION T_REGISTER {
        if (pass_number == 2) {
            expand_pseudoinstruction($1, $2, "", "", "");
        }
        current_address += 4;
    }

    /* Pseudoinstructions - rd, immediate (li) OR rs1, immediate (beqz/bnez with imm offset) */
  | PSEUDO_INSTRUCTION T_REGISTER T_COMMA T_IMMEDIATE {
        long inc = 4;
        if (strcmp($1, "li") == 0) {
            long imm = strtol($4, NULL, 0);
            if (imm > 2047 || imm < -2048) inc = 8; /* lui+addi */
        }
        if (pass_number == 2) {
            expand_pseudoinstruction($1, $2, "", "", $4);
        }
        current_address += inc;
    }

    /* Pseudoinstructions - rd, rs1 (mv, neg, not, etc.) */
  | PSEUDO_INSTRUCTION T_REGISTER T_COMMA T_REGISTER {
        if (pass_number == 2) {
            expand_pseudoinstruction($1, $2, $4, "", "");
        }
        current_address += 4;
    }

    /* Pseudoinstructions - rs1, label (beqz, bnez, blez, bgez, etc.) */
  | PSEUDO_INSTRUCTION T_REGISTER T_COMMA T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($4);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            char offset_str[32];
            snprintf(offset_str, sizeof(offset_str), "%d", offset);
            expand_pseudoinstruction($1, $2, "", "", offset_str);
        }
        current_address += 4;
    }

    /* Pseudoinstructions - rs1, rs2, label (bgt, ble, bgtu, bleu) */
  | PSEUDO_INSTRUCTION T_REGISTER T_COMMA T_REGISTER T_COMMA T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($6);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            char offset_str[32];
            snprintf(offset_str, sizeof(offset_str), "%d", offset);
            expand_pseudoinstruction($1, $2, $4, "", offset_str);
        }
        current_address += 4;
    }

    /* Pseudoinstructions - label only (j) */
  | PSEUDO_INSTRUCTION T_USETAG {
        if (pass_number == 2) {
            int target_addr = get_label_address($2);
            if (target_addr == -1) { yyerror("Undefined label"); YYABORT; }
            int offset = target_addr - current_address;
            char offset_str[32];
            snprintf(offset_str, sizeof(offset_str), "%d", offset);
            expand_pseudoinstruction($1, offset_str, "", "", "");
        }
        current_address += 4;
    }
  ;

%%


void expand_pseudoinstruction(const char* pseudo, char* rd, char* rs1, char* rs2, char* imm) {
    printf("Expanding pseudoinstruction: %s\n", pseudo);

    if (strcmp(pseudo, "nop") == 0) {
        add_symb_tab("addi", I_INSTRUCTION, "x0", "x0", "", 0);
    } else if (strcmp(pseudo, "mv") == 0) {
        add_symb_tab("addi", I_INSTRUCTION, rd, rs1, "", 0);
    } else if (strcmp(pseudo, "not") == 0) {
        add_symb_tab("xori", I_INSTRUCTION, rd, rs1, "", -1);
    } else if (strcmp(pseudo, "neg") == 0) {
        add_symb_tab("sub", R_INSTRUCTION, rd, "x0", rs1, 0);
    } else if (strcmp(pseudo, "seqz") == 0) {
        add_symb_tab("sltiu", I_INSTRUCTION, rd, rs1, "", 1);
    } else if (strcmp(pseudo, "snez") == 0) {
        add_symb_tab("sltu", R_INSTRUCTION, rd, "x0", rs1, 0);
    } else if (strcmp(pseudo, "sltz") == 0) {
        add_symb_tab("slt", R_INSTRUCTION, rd, rs1, "x0", 0);
    } else if (strcmp(pseudo, "sgtz") == 0) {
        add_symb_tab("slt", R_INSTRUCTION, rd, "x0", rs1, 0);
    } else if (strcmp(pseudo, "beqz") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("beq", B_INSTRUCTION, "", rs1 ? rs1 : rd, "x0", (int)offset);
    } else if (strcmp(pseudo, "bnez") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bne", B_INSTRUCTION, "", rs1 ? rs1 : rd, "x0", (int)offset);
    } else if (strcmp(pseudo, "blez") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bge", B_INSTRUCTION, "", "x0", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "bgez") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bge", B_INSTRUCTION, "", rs1 ? rs1 : rd, "x0", (int)offset);
    } else if (strcmp(pseudo, "bltz") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("blt", B_INSTRUCTION, "", rs1 ? rs1 : rd, "x0", (int)offset);
    } else if (strcmp(pseudo, "bgtz") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("blt", B_INSTRUCTION, "", "x0", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "bgt") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("blt", B_INSTRUCTION, "", rs2 ? rs2 : "", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "ble") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bge", B_INSTRUCTION, "", rs2 ? rs2 : "", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "bgtu") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bltu", B_INSTRUCTION, "", rs2 ? rs2 : "", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "bleu") == 0) {
        long offset = strtol(imm, NULL, 0);
        add_symb_tab("bgeu", B_INSTRUCTION, "", rs2 ? rs2 : "", rs1 ? rs1 : rd, (int)offset);
    } else if (strcmp(pseudo, "j") == 0) {
        long offset = strtol(rd, NULL, 0);
        add_symb_tab("jal", J_INSTRUCTION, "x0", "", "", (int)offset);
    } else if (strcmp(pseudo, "jr") == 0) {
        add_symb_tab("jalr", I_INSTRUCTION, "x0", rd, "", 0);
    } else if (strcmp(pseudo, "ret") == 0) {
        add_symb_tab("jalr", I_INSTRUCTION, "x0", "x1", "", 0);
    } else if (strcmp(pseudo, "li") == 0) {
        long imm_val = strtol(imm, NULL, 0);
        if (imm_val >= -2048 && imm_val <= 2047) {
            add_symb_tab("addi", I_INSTRUCTION, rd, "x0", "", (int)imm_val);
        } else {
            int upper = (imm_val + 0x800) >> 12;
            int lower = imm_val & 0xfff;
            if (lower > 2047) lower = lower - 4096;
            add_symb_tab("lui", U_INSTRUCTION, rd, "", "", upper);
            add_symb_tab("addi", I_INSTRUCTION, rd, rd, "", lower);
        }
    }
}

void reset_for_second_pass() {
    current_address = 0;
    pass_number = 2;
    rewind(yyin);
}

void generate_output_files() {
    if (!hex_filename || !bin_filename) return;

    FILE* hex_file = fopen(hex_filename, "w");
    FILE* bin_file = fopen(bin_filename, "w");

    if (!hex_file || !bin_file) {
        yyerror("Error opening output files");
        return;
    }

    printf("Generating output files: %s and %s\n", hex_filename, bin_filename);

    for (int i = 0; i < symbol_count_func(); i++) {
        uint32_t machine_code = generate_machine_code(i);
        fprintf(hex_file, "%08x\n", machine_code);
        for (int bit = 31; bit >= 0; bit--) fprintf(bin_file, "%d", (machine_code >> bit) & 1);
        fprintf(bin_file, "\n");
    }

    fclose(hex_file);
    fclose(bin_file);
}

void yyerror(const char *s) { fprintf(stderr, "Error: %s\n", s); }

int main(int argc, char* argv[]) {
    if (argc != 4) { fprintf(stderr, "Usage: %s input.asm output.hex output.bin\n", argv[0]); return 1; }
    input_filename = argv[1]; hex_filename = argv[2]; bin_filename = argv[3];
    FILE* f = fopen(input_filename, "r"); if (!f) { perror(input_filename); return 1; }
    yyin = f;
    printf("=== FIRST PASS ===\n"); pass_number = 1; current_address = 0; yyparse();
    reset_for_second_pass();
    printf("\n=== SECOND PASS ===\n"); yyparse();
    printf("\n=== GENERATING OUTPUT ===\n"); generate_output_files();
    print_table(); cleanup_symbol_table(); fclose(f);
    printf("Assembly completed successfully!\n"); return 0;
}
