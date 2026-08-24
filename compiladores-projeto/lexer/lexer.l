%{
#include "parser.tab.h"
%}

%%

[0-9]+          { yylval = atoi(yytext); return NUM; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return DIVIDE; }
"=="            { return EQ; }
"!="            { return NEQ; }
">="            { return GTE; }
"<="            { return LTE; }
">"             { return GT; }
"<"             { return LT; }
"="             { return ASSIGN; }
"("             { return LPAREN; }
")"             { return RPAREN; }
":"             { return COLON; }
","             { return COMMA; }
"/*"([^*]|\*+[^*/])*\*+"/" { /* Ignorar comentários em bloco */ }
"#"[^\n]*       { /* Ignorar comentários de linha */ }
[ \t\n]+        { /* Ignorar espaços em branco */ }
.               { printf("Caractere inválido: %s\n", yytext); }

%%
