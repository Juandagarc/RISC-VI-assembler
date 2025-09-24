# Makefile para compilar Bison (parser.y) + Flex (lexical_analyzer.l)
# macOS: no requiere -lfl porque usamos %option noyywrap en el lexer

FLEX   := flex
BISON  := bison
CC     := cc
CFLAGS ?= -O2 -Wall -Wextra -Wno-sign-compare

BIN    := assembler
LEX    := lexical_analyzer.l
YACC   := parser.y
LEXGEN := lex.yy.c
YACCC  := parser.tab.c
YACCH  := parser.tab.h

.PHONY: all clean run

all: $(BIN)

$(YACCC) $(YACCH): $(YACC)
	$(BISON) -d -o $(YACCC) $(YACC)

$(LEXGEN): $(LEX) $(YACCH)
	$(FLEX) -o $(LEXGEN) $(LEX)

$(BIN): $(YACCC) $(LEXGEN)
	$(CC) $(CFLAGS) -o $@ $(YACCC) $(LEXGEN)

run: $(BIN)
	@echo "Ejecutando con test.asm (puedes editarlo):"
	./$(BIN) < test.asm

clean:
	rm -f $(BIN) $(LEXGEN) $(YACCC) $(YACCH) parser.output
