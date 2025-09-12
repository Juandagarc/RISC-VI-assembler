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
REGNUM  [xX]{DIGIT}+

%%

"//".*               { /* ignore */ }
"#".*                { /* ignore */ }
{WS}                  { /* ignore */ }
{EOL}                 { return T_EOL; }
","                   { return T_COMMA; }


add|sub|sll|slt|sltu|xor|srl|sra|or|and { // R-type instructions
                                        yylval.iValue = strdup(yytext); return T_INSTRUCTION;
                                        }


{REGNUM}              { yylval.iValue = strdup(yytext); return T_REGISTER; }
{ID}                  { yylval.iValue = strdup(yytext); return T_USETAG; }
.                     { /* ignore */ }


%%
