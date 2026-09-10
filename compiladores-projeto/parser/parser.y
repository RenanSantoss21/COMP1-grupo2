%{
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

/* Linha do token que o parser está examinando (mantida pelo lexer). */
extern int linha_token;

/* Quantos erros sintáticos foram reportados. Com a recuperação de erros o
   yyparse() pode terminar com sucesso mesmo depois de erros, então é este
   contador que decide o código de saída do compilador. */
int erros_sintaticos = 0;

int yylex(void);
void yyerror(const char *s);
%}

 /* Mensagens de erro detalhadas: informam o token encontrado e os esperados.
    O LAC garante que a lista de tokens esperados esteja correta. */
%define parse.error verbose
%define parse.lac full

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
    | erro_linha
    | erro_linha INDENT comandos DEDENT
    | erro_indentacao comandos DEDENT
    ;

 /* Recuperação de erros: ao encontrar um erro, o Bison desempilha estados até
    um ponto em que um comando pode começar, empilha o token especial `error` e
    descarta tokens até achar um que continue uma das regras abaixo.

    - erro_linha: descarta o resto da linha com erro. Se a linha era o
      cabeçalho de um bloco (ex.: `if x > 1` sem ':'), o bloco indentado que
      vem em seguida ainda é analisado normalmente.
    - erro_indentacao: bloco indentado sem cabeçalho (INDENT inesperado); o
      conteúdo do bloco é analisado normalmente.

    yyerrok sai do modo de recuperação na hora, para que o próximo erro também
    seja reportado (sem ele o Bison silencia erros nos 3 tokens seguintes).
    yyclearin descarta o lookahead, se houver, para recomeçar com um token novo. */
erro_linha:
      error NEWLINE     { yyerrok; yyclearin; }
    ;

erro_indentacao:
      error INDENT      { yyerrok; yyclearin; }
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
    /* O modo verbose produz "syntax error, unexpected X, expecting Y"; o
       prefixo é redundante com o nosso "Erro sintático". */
    const char *prefixo = "syntax error, ";
    if (strncmp(s, prefixo, strlen(prefixo)) == 0)
        s += strlen(prefixo);

    erros_sintaticos++;
    fprintf(stderr, "Erro sintático na linha %d: %s\n", linha_token, s);
}

int main(void) {
    int resultado = yyparse();

    if (erros_sintaticos > 0) {
        fprintf(stderr, "%d erro(s) sintático(s) encontrado(s).\n", erros_sintaticos);
        return 1;
    }
    return resultado;
}
