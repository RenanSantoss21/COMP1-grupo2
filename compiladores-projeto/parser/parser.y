%{
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "src/ast.h"

/* Linha do token que o parser está examinando (mantida pelo lexer). */
extern int linha_token;

/* Variável global que armazena a raiz da árvore. */
NoAST *raiz_ast = NULL;

/* Quantos erros sintáticos foram reportados. Com a recuperação de erros o
   yyparse() pode terminar com sucesso mesmo depois de erros, então é este
   contador que decide o código de saída do compilador. */
int erros_sintaticos = 0;

int yylex(void);
void yyerror(const char *s);

NoAST* criar_lista(NoAST* primeiro) {
    NoAST** arr = malloc(sizeof(NoAST*));
    arr[0] = primeiro;
    return criar_no_bloco(arr, 1);
}

/* Adiciona um nó no final do vetor e realoca o bloco. */
NoAST* adicionar_lista(NoAST* lista, NoAST* novo) {
      if (!lista) return criar_lista(novo);
      if (!novo) return lista;

      lista->num_argumentos++;
      lista->argumentos = realloc(lista->argumentos, lista->num_argumentos * sizeof(NoAST*));
      lista->argumentos[lista->num_argumentos - 1] = novo;
      return lista;
}
%}

 /* Mensagens de erro detalhadas: informam o token encontrado e os esperados.
    O LAC garante que a lista de tokens esperados esteja correta. */
%define parse.error verbose
%define parse.lac full

%union {
    int ival;
    double dval;
    char *sval;
    struct noAST *ast;
}

%token <ival> NUM_INT
%token <dval> NUM_FLOAT
%token <sval> STRING_LITERAL
%token <sval> ID
%token PLUS MINUS TIMES DIVIDE LPAREN RPAREN COLON LCOLCH RCOLCH
%token ASSIGN EQ NEQ GTE LTE GT LT COMMA
%token IF ELSE ELIF WHILE FOR IN RANGE DEF RETURN PRINT INPUT
%token FALSE TRUE NOT AND OR
%token INDENT DEDENT NEWLINE

%type <ast> programa comandos comando comando_simples comando_composto bloco
%type <ast> erro_linha erro_indentacao
%type <ast> lista_elif senao_opt parametros_opt parametros argumentos_opt expressao

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
      %empty { raiz_ast = NULL; }
    | comandos {
       if ($1 && $1->tipo == NO_BLOCO) {
              raiz_ast = criar_no_programa($1->argumentos, $1->num_argumentos);
              free($1); /* Libera o bloco temporário, mantendo o array de argumentos interno */
          } else if ($1) {
              NoAST *arr[] = {$1};
              raiz_ast = criar_no_programa(arr, 1);
          } else {
              raiz_ast = NULL;
          }
          $$ = raiz_ast;   
    }
    ;

comandos:
      comando { $$ = criar_lista($1); }
    | comandos comando { $$ = adicionar_lista($1, $2); }
    ;

comando:
      comando_simples NEWLINE { $$ = $1; }
    | comando_composto { $$ = $1; }
    | erro_linha { $$ = NULL; }
    | erro_linha INDENT comandos DEDENT { $$ = $3; }
    | erro_indentacao comandos DEDENT { $$ = $2; }
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
      error NEWLINE     { yyerrok; yyclearin; $$ = NULL; }
    ;

erro_indentacao:
      error INDENT      { yyerrok; yyclearin; $$ = NULL; }
    ;

 /* Bloco indentado: o NEWLINE fecha a linha do cabeçalho (if/while/def/...),
    o INDENT abre o bloco e o DEDENT o fecha. */
bloco:
      NEWLINE INDENT comandos DEDENT { $$ = $3; } /* Repassa a lista completa. */
    ;

comando_simples:
      expressao { $$ = $1; }
    | RETURN { $$ = criar_no_return(NULL, linha_token); }
    | RETURN expressao { $$ = criar_no_return($2, linha_token); }
    | PRINT LPAREN argumentos_opt RPAREN {
        int n_args = 0;
          NoAST **args = NULL;
          if ($3) {
              if ($3->tipo == NO_BLOCO) {
                  n_args = $3->num_argumentos;
                  args = $3->argumentos;
              } else {
                  n_args = 1;
                  args = malloc(sizeof(NoAST*));
                  args[0] = $3;
              }
          }
          $$ = criar_no_print(args, n_args, linha_token);
    }
    ;

comando_composto:
      IF expressao COLON bloco lista_elif senao_opt {
          int n_elifs = 0;
          NoAST **elifs = NULL;
          if ($5 && $5->tipo == NO_BLOCO) {
              n_elifs = $5->num_argumentos;
              elifs = $5->argumentos;
          }
          $$ = criar_no_if($2, $4, elifs, n_elifs, $6, linha_token);
          if ($5) free($5); /* Libera a casca do wrapper NO_BLOCO */

      }
    | WHILE expressao COLON bloco { $$ = criar_no_while($2, $4, linha_token); }
    | FOR ID IN RANGE LPAREN argumentos_opt RPAREN COLON bloco { $$ = criar_no_for($2, $6, $9, linha_token); }
    | DEF ID LPAREN parametros_opt RPAREN COLON bloco {
        int n_params = 0;
        NoAST **params = NULL;
        if ($4 && $4->tipo == NO_BLOCO) {
            n_params = $4->num_argumentos;
            params = $4->argumentos;
        }
        $$ = criar_no_funcdef($2, params, n_params, $7, linha_token);
        if ($4) free($4);
    }
    ;

lista_elif:
      %empty { $$ = NULL; }
    | lista_elif ELIF expressao COLON bloco { $$ = adicionar_lista($1, criar_no_elif($3, $5, linha_token)); }
    ;

senao_opt:
      %empty            %prec LOWER_THAN_ELSE { $$ = NULL; }
    | ELSE COLON bloco { $$ = $3; }
    ;

parametros_opt:
      %empty { $$ = NULL; }
    | parametros { $$ = $1; }
    ;

parametros:
      ID { $$ = criar_lista(criar_no_id($1, linha_token)); }
    | parametros COMMA ID { $$ = adicionar_lista($1, criar_no_id($3, linha_token)); }
    ;

 /* Uma expressão já cobre listas separadas por vírgula (regra expressao COMMA
    expressao), então a lista de argumentos é apenas uma expressão opcional. */
argumentos_opt:
      %empty { $$ = NULL; }
    | expressao { $$ = $1; }
    ;

expressao:
      expressao PLUS expressao   { $$ = criar_no_binop(OP_SOMA, $1, $3, linha_token); }
    | expressao MINUS expressao  { $$ = criar_no_binop(OP_SUB, $1, $3, linha_token); }
    | expressao ASSIGN expressao {
          /* Verifica se o lado esquerdo é um ID válido */
          char *nome = ($1 && $1->tipo == NO_ID) ? $1->nome : "invalido";
          $$ = criar_no_assign(nome, $3, linha_token);
      }
    | expressao EQ expressao     { $$ = criar_no_binop(OP_EQ, $1, $3, linha_token); }
    | expressao NEQ expressao    { $$ = criar_no_binop(OP_NEQ, $1, $3, linha_token); }
    | expressao GTE expressao    { $$ = criar_no_binop(OP_GTE, $1, $3, linha_token); }
    | expressao LTE expressao    { $$ = criar_no_binop(OP_LTE, $1, $3, linha_token); }
    | expressao GT expressao     { $$ = criar_no_binop(OP_GT, $1, $3, linha_token); }
    | expressao LT expressao     { $$ = criar_no_binop(OP_LT, $1, $3, linha_token); }
    | expressao COMMA expressao  {
          /* Converte expressões divididas por vírgulas em uma lista/tupla (NO_BLOCO) */
          if ($1 && $1->tipo == NO_BLOCO) {
              $$ = adicionar_lista($1, $3);
          } else {
              $$ = adicionar_lista(criar_lista($1), $3);
          }
      }
    | expressao TIMES expressao  { $$ = criar_no_binop(OP_MULT, $1, $3, linha_token); }
    | expressao DIVIDE expressao { $$ = criar_no_binop(OP_DIV, $1, $3, linha_token); }
    | expressao AND expressao    { $$ = criar_no_binop(OP_AND, $1, $3, linha_token); }
    | expressao OR expressao     { $$ = criar_no_binop(OP_OR, $1, $3, linha_token); }
    | NOT expressao              { $$ = criar_no_unaryop(OP_NOT, $2, linha_token); }
    | expressao IN expressao     { $$ = criar_no_binop(OP_IN, $1, $3, linha_token); }
    | MINUS expressao %prec NOT  { $$ = criar_no_unaryop(OP_NEG, $2, linha_token); }
    | LPAREN expressao RPAREN    { $$ = $2; }
    | LCOLCH RCOLCH              { $$ = criar_no_bloco(NULL, 0); }
    | LCOLCH expressao RCOLCH    { $$ = ($2 && $2->tipo == NO_BLOCO) ? $2 : criar_lista($2); }
    | INPUT LPAREN argumentos_opt RPAREN {
          /* Para `input()`, enviamos a expressão passada como prompt, se houver */
          $$ = criar_no_input($3, linha_token);
      }
    | ID LPAREN argumentos_opt RPAREN {
          int n_args = 0;
          NoAST **args = NULL;
          if ($3) {
              if ($3->tipo == NO_BLOCO) {
                  n_args = $3->num_argumentos;
                  args = $3->argumentos;
              } else {
                  n_args = 1;
                  args = malloc(sizeof(NoAST*));
                  args[0] = $3;
              }
          }
          $$ = criar_no_funccall($1, args, n_args, linha_token);
      }
    | FALSE                      { $$ = criar_no_bool(0, linha_token); }
    | TRUE                       { $$ = criar_no_bool(1, linha_token); }
    | NUM_INT                    { $$ = criar_no_int($1, linha_token); }
    | NUM_FLOAT                  { $$ = criar_no_float($1, linha_token); }
    | STRING_LITERAL             { $$ = criar_no_string($1, linha_token); }
    | ID                         { $$ = criar_no_id($1, linha_token); }
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

    /* Critério de aceite final: imprimir a árvore estruturada se o parsing tiver sucesso */
    if (raiz_ast != NULL) {
        printf("\n--- Árvore Sintática Abstrata Gerada ---\n");
        imprimir_ast(raiz_ast, 0);
    }

    return resultado;
}
