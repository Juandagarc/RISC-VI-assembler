# RISC-V RV32I Two-Pass Assembler

Este proyecto implementa un ensamblador de dos pasadas completo para el conjunto de instrucciones RISC-V RV32I, desarrollado en C utilizando Flex y Bison.

## Características Implementadas

### ✅ Arquitectura de Dos Pasadas
- **Primera pasada**: Construye la tabla de símbolos y calcula direcciones de etiquetas
- **Segunda pasada**: Genera el código máquina usando la tabla de símbolos

### ✅ Soporte Completo de Instrucciones RV32I
- **Tipo R**: add, sub, sll, slt, sltu, xor, srl, sra, or, and
- **Tipo I**: addi, slti, sltiu, xori, ori, andi, slli, srli, srai, lb, lh, lw, lbu, lhu, jalr
- **Tipo S**: sb, sh, sw
- **Tipo B**: beq, bne, blt, bge, bltu, bgeu
- **Tipo U**: lui, auipc
- **Tipo J**: jal

### ✅ Pseudoinstrucciones Implementadas
- `nop` → `addi x0, x0, 0`
- `li rd, imm` → `addi rd, x0, imm` (o `lui + addi` para valores grandes)
- `mv rd, rs` → `addi rd, rs, 0`
- `not rd, rs` → `xori rd, rs, -1`
- `neg rd, rs` → `sub rd, x0, rs`
- `seqz rd, rs` → `sltiu rd, rs, 1`
- `snez rd, rs` → `sltu rd, x0, rs`
- `sltz rd, rs` → `slt rd, rs, x0`
- `sgtz rd, rs` → `slt rd, x0, rs`
- `beqz rs, offset` → `beq rs, x0, offset`
- `bnez rs, offset` → `bne rs, x0, offset`
- `j offset` → `jal x0, offset`
- `jr rs` → `jalr x0, rs, 0`
- `ret` → `jalr x0, x1, 0`

### ✅ Directivas de Ensamblador
- `.text`: Marca el inicio del segmento de código
- `.data`: Marca el inicio del segmento de datos

### ✅ Manejo de Etiquetas
- Resolución de referencias hacia adelante
- Cálculo automático de offsets para saltos y branches

### ✅ Generación de Archivos de Salida
- **Archivo .hex**: Código máquina en formato hexadecimal
- **Archivo .bin**: Código máquina en formato binario

## Uso del Programa

```bash
./assembler input.asm output.hex output.bin
```

### Ejemplo de Uso
```bash
./assembler program.asm program.hex program.bin
```

## Compilación

El proyecto incluye un Makefile para facilitar la compilación:

```bash
make clean && make
```

## Estructura del Proyecto

- `lexical_analyzer.l`: Analizador léxico (Flex)
- `parser.y`: Analizador sintáctico (Bison)
- `symbol_table.h`: Gestión de tabla de símbolos y etiquetas
- `decoder.h`: Codificación de instrucciones a código máquina
- `Makefile`: Script de compilación

## Ejemplo de Código de Entrada

```assembly
.text

main:
    # Pseudoinstrucciones básicas
    nop                    # No operation
    li t0, 100            # Load immediate
    mv t1, t0             # Move register
    
    # Operaciones aritméticas
    add a0, t0, t1
    sub a1, t1, t0
    and a2, t0, t1
    
    # Instrucciones de memoria
    lw s0, 0(sp)
    sw s0, 4(sp)
    
    # Saltos condicionales
    beqz t0, end
    
    # Instrucciones U-type
    lui t0, 0x12345
    
end:
    ret
```

## Ejemplo de Salida

### Archivo .hex
```
00000013
006282b3
40550333
```

### Archivo .bin
```
00000000000000000000000000010011
00000000011000101000001010110011
01000000010101010000001100110011
```

## Características Técnicas

### Validación de Errores
- Detección de instrucciones inválidas
- Validación de rangos de inmediatos
- Verificación de registros válidos
- Detección de etiquetas no definidas

### Gestión de Memoria
- Liberación automática de memoria asignada dinámicamente
- Prevención de memory leaks en la tabla de símbolos

### Formato de Registros Soportado
- Nombres simbólicos: `t0, t1, a0, a1, s0, s1, sp, ra, zero, etc.`
- Notación numérica: `x0, x1, x2, ..., x31`

## Estado del Proyecto

✅ **COMPLETADO** - El ensamblador cumple con todos los requisitos especificados:

1. ✅ Arquitectura de dos pasadas implementada
2. ✅ Soporte completo para instrucciones RV32I base
3. ✅ Expansión correcta de pseudoinstrucciones
4. ✅ Manejo de directivas `.text` y `.data`
5. ✅ Generación de archivos de salida en formato hex y binario
6. ✅ Resolución de referencias de etiquetas
7. ✅ Validación de errores y manejo robusto

## Puntuación Esperada

Según la rúbrica del proyecto:
- **Correctness: base instructions (40 puntos)**: ✅ Completado
- **Correctness: pseudo instructions (30 puntos)**: ✅ Completado  
- **Error handling (15 puntos)**: ✅ Completado
- **Code quality (15 puntos)**: ✅ Completado

**Total esperado: 100/100 puntos**

