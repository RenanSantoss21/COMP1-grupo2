%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

%token NUM PLUS MINUS TIMES DIVIDE LPAREN RPAREN
%token EQ NEQ GT LT GTE LTE ASSIGN COLON COMMA

%%

expressao:
    expressao PLUS expressao
  | expressao MINUS expressao
  | expressao TIMES expressao
  | expressao DIVIDE expressao
  | LPAREN expressao RPAREN%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);
void yyerror(const char *s);
%}

%token NUM ID
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN COLON LCOLCH RCOLCH
%token ASSIGN EQ NEQ GTE LTE GT LT COMMA
%token IF ELSE ELIF WHILE FOR IN RANGE DEF RETURN PRINT INPUT 
%token FALSE TRUE NOT AND OR 
%token INDENT DEDENT NEWLINE

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
    | expressao  AND expressao   
    | expressao OR expressao 
    | NOT expressao 
    | expressao IN expressao
    | LPAREN expressao RPAREN
    | LCOLCH expressao RCOLCH
    | INPUT LPAREN RPAREN
    | FALSE
    | TRUE
    | NUM
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

  | NUM
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático: %s\n", s);
}

int main(void) {
    yyparse();
    return 0;
}
