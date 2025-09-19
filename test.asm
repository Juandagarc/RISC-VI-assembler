// Pruebas completas R, I, S, B, U para el ensamblador actual
// Formato soportado por el parser:
//  - R:  inst rd, rs1, rs2
//  - I:  inst rd, rs1, imm
//  - S:  inst rs2, rs1, imm
//  - B:  inst rs1, rs2, label|imm
//  - U:  inst rd, imm

// ==== R-type ====
add t2, x30, x6
add t1, a0, a1
sub t2, t1, t0
xor t3, a0, a0
sll t4, t1, t0
slt s0, s1, s2
sltu s1, s2, s3
srl a0, a1, a2
sra a1, a2, a3
or  a2, a3, a4
and a3, a4, a5

# ==== I-type ====
addi t0, t1, 42          # decimal
xori a0, a1, -13         # negativo
ori a1, a2, 0x7F        # hex
andi a2, a3, 0           # cero
slli a3, a4, 3           # shift
srli a4, a5, 1
srai a5, a6, 4
lb   t0, t1, 8
lh   t1, t2, -4
lw   t2, t3, 0x10
lbu  a0, a1, 0
lhu  a1, a2, 0x20
jalr ra, t0, 0

// ==== S-type (rs2, rs1, imm) ====
sb t0, t1, 0
sh a1, a0, -16
sw x5, x6, 0x20

// ==== B-type (rs1, rs2, label|imm) ====
beq t0, t1, target
bne a0, a1, -8
blt s0, s1, 0x10
bge x2, x3, 0
bltu t2, t3, L1
bgeu x0, x0, 4

// ==== U-type (rd, imm) ====
lui  t0, 0x12345
auipc a1, 2048

// Etiquetas usadas como operandos en ramas (no se declaran como definiciones)
// target, L1
