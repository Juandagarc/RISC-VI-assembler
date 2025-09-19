%{
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
%}

%option noyywrap
%option nounput
%option noinput

WS      [ \t]+
EOL     (\r\n|\n|\r)
DIGIT   [0-9]
ID      [A-Za-z_][A-Za-z0-9_]*
REGNUM  (x([0-9]|1[0-9]|2[0-9]|3[0-1])|zero|ra|sp|gp|tp|t[0-6]|s[0-9]|s1[0-1]|a[0-7]|fp)
INM     (-?0[xX][0-9a-fA-F]+)|(-?[0-9]+)

%%

"//".*               { /* ignore */ }
"#".*                { /* ignore */ }
{WS}                  { /* ignore */ }
{EOL}                 { return T_EOL; }
","                   { return T_COMMA; }


("add"|"sub"|"sll"|"slt"|"sltu"|"xor"|"srl"|"sra"|"or"|"and") {
                                        yylval.sValue = strdup(yytext);
                                        return R_T_INSTRUCTION;
                                        }


("addi"|"slti"|"sltiu"|"xori"|"ori"|"andi"|"slli"|"srli"|"srai"|"lb"|"lh"|"lw"|"lbu"|"lhu"|"jalr") {
                                        yylval.sValue = strdup(yytext);
                                        return I_T_INSTRUCTION;
                                        }

("sb"|"sh"|"sw")                      { yylval.sValue = strdup(yytext); return S_T_INSTRUCTION; }

("beq"|"bne"|"blt"|"bge"|"bltu"|"bgeu") { yylval.sValue = strdup(yytext); return B_T_INSTRUCTION; }

("lui"|"auipc")                        { yylval.sValue = strdup(yytext); return U_T_INSTRUCTION; }

{REGNUM}              { yylval.sValue = strdup(yytext); return T_REGISTER; }
{ID}                  { yylval.sValue = strdup(yytext); return T_USETAG; }
{INM}                 { yylval.sValue = strdup(yytext); return T_IMMEDIATE; }
.                     { /* ignore */ }


%%