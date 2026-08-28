%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>
int indent_stack[100];
int nivel_atual = 0;

void init_stack() {
    indent_stack[0] = 0;
}

%}

%%

"False" {return FALSE;}
"True" {return TRUE; }
"not"  {return NOT; }
"and" {return AND; }
"or"  {return OR; }
"print" {return PRINT; }
"if" {return IF; }
"else" {return ELSE; }
"elif" {return ELIF;}
"while" {return WHILE; }
"for" {return FOR; }
"in" {return IN; }
"def" {return DEF;}
"return" {return RETURN;}
"range" {return RANGE; }
"input" {return INPUT; }




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
"["             { return LCOLCH; }
"]"             { return RCOLCH; }
":"             { return COLON; }
","             { return COMMA; }

"/*"([^*]|\*+[^*/])*\*+"/" { /* Ignorar comentários em bloco */ }
"#"[^\n]*       { /* Ignorar comentários de linha */ }

[a-zA-Z_][a-zA-Z0-9_]* { return ID; }

 /* Literais */
[0-9]+"."[0-9]*([eE][+-]?[0-9]+)? |
"."[0-9]+([eE][+-]?[0-9]+)?       | 
[0-9]+[eE][+-]?[0-9]+             { 
    yylval.dval = atof(yytext);
    return NUM_FLOAT; 
}

[0-9]+ { 
    yylval.ival = atoi(yytext);
    return NUM_INT;
}

 /* Strings */
\"([^\"\\]|\\.)*\" {
    yylval.sval = strdup(yytext);
    return STRING_LITERAL;
}

\x27([^\x27\\]|\\.)*\x27 {
    yylval.sval = strdup(yytext);
    return STRING_LITERAL;
}

 /* Indentação */
\n[ \t]*    {
    int espacos = yyleng - 1; 

    if (espacos > indent_stack[nivel_atual]) {
        nivel_atual++;
        indent_stack[nivel_atual] = espacos;
        return INDENT; 
    }
    
    else if (espacos < indent_stack[nivel_atual]) {
        while (nivel_atual > 0 && indent_stack[nivel_atual] > espacos) {
            nivel_atual--;
    
            return DEDENT; 
        }
    }
    
    else {
        return NEWLINE; 
    }
}

[ \t]+      {  }

.               { printf("Caractere inválido: %s\n", yytext); }

%%

