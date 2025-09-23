.text

main:
    nop
    li t0, 100
    mv t1, t0
    not t2, t1
    neg t3, t2
    seqz t4, t0
    snez t5, t1
    sltz t6, t2
    sgtz s0, t3
    add a0, t0, t1
    sub a1, t1, t0
    and a2, t0, t1
    or a3, a0, a1
    xor a4, a2, a3
    sll s1, t0, t1
    srl s2, t1, t0
    sra s3, a0, a1
    slt s4, t0, t1
    sltu s5, t1, t0
    addi sp, sp, -16
    andi t2, t0, 255
    ori t3, t1, 15
    xori t4, t2, 127
    slti s6, t0, 50
    sltiu s7, t1, 200
    slli s8, t0, 2
    srli s9, t1, 1
    srai s10, a0, 3
    lw s11, 0(sp)
    lh ra, 2(sp)
    lb gp, 1(sp)
    lhu tp, 4(sp)
    lbu fp, 5(sp)
    sw s11, 8(sp)
    sh ra, 10(sp)
    sb gp, 9(sp)
    beq t0, t1, skip
    bne t1, t2, skip
    blt t2, t3, skip
    bge t3, t4, skip
    bltu t4, t5, skip
    bgeu t5, t6, skip
    beqz t0, skip
    bnez t1, skip
    blez t2, skip
    bgez t3, skip
    bltz t4, skip
    bgtz t5, skip
    bgt t0, t1, skip
    ble t1, t2, skip
    bgtu t2, t3, skip
    bleu t3, t4, skip
    lui t5, 305
    auipc t6, 256
    jal ra, subroutine
    j end

skip:
    jalr ra, t0, 0

subroutine:
    jr ra
    ret

end:
    nop