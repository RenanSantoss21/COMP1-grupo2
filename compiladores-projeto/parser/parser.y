%{
#include <stdio.h>
#include <stdlib.h>

extern int linha_atual;

int yylex(void);
void yyerror(const char *s);
%}

%union {
    int ival;
    double dval;
    char *sval;
}

%token <ival> NUM_INT
%token <dval> NUM_FLOAT
%token <sval> STRING_LITERAL
%token ID
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN COLON LCOLCH RCOLCH
%token ASSIGN EQ NEQ GTE LTE GT LT COMMA
%token IF ELSE ELIF WHILE FOR IN RANGE DEF RETURN PRINT INPUT
%token FALSE TRUE NOT AND OR
%token INDENT DEDENT NEWLINE

%right ASSIGN
%left COMMA
%left OR
%left AND
%right NOT
%nonassoc EQ NEQ GTE LTE GT LT IN
%left PLUS MINUS
%left TIMES DIVIDE

%precedence LOWER_THAN_ELSE
%precedence ELSE ELIF

%%

programa:
      %empty
    | comandos
    ;

comandos:
      comando
    | comandos comando
    ;

comando:
      comando_simples NEWLINE
    | comando_composto
    ;

 /* Bloco indentado: o NEWLINE fecha a linha do cabeçalho (if/while/def/...),
    o INDENT abre o bloco e o DEDENT o fecha. */
bloco:
      NEWLINE INDENT comandos DEDENT
    ;

comando_simples:
      expressao
    | RETURN
    | RETURN expressao
    | PRINT LPAREN argumentos_opt RPAREN
    ;

comando_composto:
      IF expressao COLON bloco lista_elif senao_opt
    | WHILE expressao COLON bloco
    | FOR ID IN RANGE LPAREN argumentos_opt RPAREN COLON bloco
    | DEF ID LPAREN parametros_opt RPAREN COLON bloco
    ;

lista_elif:
      %empty
    | lista_elif ELIF expressao COLON bloco
    ;

senao_opt:
      %empty            %prec LOWER_THAN_ELSE
    | ELSE COLON bloco
    ;

parametros_opt:
      %empty
    | parametros
    ;

parametros:
      ID
    | parametros COMMA ID
    ;

 /* Uma expressão já cobre listas separadas por vírgula (regra expressao COMMA
    expressao), então a lista de argumentos é apenas uma expressão opcional. */
argumentos_opt:
      %empty
    | expressao
    ;

expressao:
      expressao PLUS expressao
    | expressao MINUS expressao
    | expressao ASSIGN expressao
    | expressao EQ expressao
    | expressao NEQ expressao
    | expressao GTE expressao
    | expressao LTE expressao
    | expressao GT expressao
    | expressao LT expressao
    | expressao COMMA expressao
    | expressao TIMES expressao
    | expressao DIVIDE expressao
    | expressao AND expressao
    | expressao OR expressao
    | NOT expressao
    | expressao IN expressao
    | MINUS expressao %prec NOT
    | LPAREN expressao RPAREN
    | LCOLCH RCOLCH
    | LCOLCH expressao RCOLCH
    | INPUT LPAREN argumentos_opt RPAREN
    | ID LPAREN argumentos_opt RPAREN
    | FALSE
    | TRUE
    | NUM_INT
    | NUM_FLOAT
    | STRING_LITERAL
    | ID
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático na linha %d: %s\n", linha_atual, s);
}

int main(void) {
    return yyparse();
}
