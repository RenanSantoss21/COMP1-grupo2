%{
#include <stdio.h>
#include <stdlib.h>

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
    comandos
;

comandos:
     comando
    |comandos comando
    ;

comando:
      expressao
    | IF expressao COLON comando
    | ELSE COLON comando
    | ELIF expressao COLON comando 
    | WHILE expressao COLON comando 
    | FOR ID IN RANGE LPAREN expressao RPAREN COLON comando
    | PRINT LPAREN expressao RPAREN
    | RETURN expressao 
    | DEF ID LPAREN  RPAREN COLON comando
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
    | LPAREN expressao RPAREN
    | LCOLCH expressao RCOLCH
    | INPUT LPAREN RPAREN
    | FALSE
    | TRUE
    | NUM_INT
    | NUM_FLOAT
    | STRING_LITERAL
    | ID
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}
