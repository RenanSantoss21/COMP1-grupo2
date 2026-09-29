%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* O scanner gerado pelo flex é renomeado para scanner(); yylex() (no fim do
   arquivo) é um invólucro que primeiro esvazia a fila de tokens de indentação. */
#define YY_DECL int scanner(void)

#define MAX_NIVEIS 100

int linha_atual = 1;

/* Linha do último token entregue ao parser, usada nas mensagens de erro
   sintático. Não dá para usar linha_atual (nem yylineno): o token NEWLINE
   consome a quebra de linha, as linhas em branco seguintes e a indentação da
   próxima linha, então quando o parser reclama do NEWLINE o contador já está
   apontando para a linha de baixo. */
int linha_token = 1;

/* Executado antes da ação de toda regra: registra a linha onde o token começa. */
#define YY_USER_ACTION linha_token = linha_atual;

static int indent_stack[MAX_NIVEIS] = {0};
static int nivel_atual = 0;

/* Fila de INDENT/DEDENT gerados por uma quebra de linha. Eles precisam ser
   entregues ao parser DEPOIS do NEWLINE daquela linha. */
static int fila[MAX_NIVEIS];
static int fila_ini = 0, fila_fim = 0;

/* A linha lógica atual já produziu algum token "de verdade"? Evita emitir
   NEWLINE para linhas em branco ou só com comentário. */
static int linha_tem_token = 0;
static int fim_tratado = 0;

static void enfileirar(int token) {
    if (fila_fim < MAX_NIVEIS) fila[fila_fim++] = token;
}

static int fila_vazia(void) { return fila_ini == fila_fim; }

static int desenfileirar(void) {
    int token = fila[fila_ini++];
    if (fila_ini == fila_fim) fila_ini = fila_fim = 0;
    return token;
}

/* Entrega o próximo token da fila ajustando a linha: o INDENT pertence à
   linha recém-indentada; o DEDENT mantém a linha do último token, onde está
   o bloco que acabou de ser fechado. */
static int token_da_fila(void) {
    int token = desenfileirar();
    if (token == INDENT) linha_token = linha_atual;
    return token;
}

/* Compara a indentação da nova linha com o topo da pilha e enfileira
   INDENT (um) ou DEDENT (quantos forem necessários). */
static void processar_indentacao(int espacos) {
    if (espacos > indent_stack[nivel_atual]) {
        if (nivel_atual + 1 < MAX_NIVEIS) {
            indent_stack[++nivel_atual] = espacos;
            enfileirar(INDENT);
        }
        return;
    }

    while (nivel_atual > 0 && espacos < indent_stack[nivel_atual]) {
        nivel_atual--;
        enfileirar(DEDENT);
    }

    if (espacos != indent_stack[nivel_atual]) {
        fprintf(stderr, "Erro léxico: indentação inconsistente na linha %d\n", linha_atual);
    }
}

/* Fecha os blocos abertos e devolve o terminador da última linha lógica. */
static int tratar_fim_de_arquivo(void) {
    fim_tratado = 1;
    while (nivel_atual > 0) {
        nivel_atual--;
        enfileirar(DEDENT);
    }
    if (linha_tem_token) {
        linha_tem_token = 0;
        return NEWLINE;
    }
    return 0;
}
%}

%option noyywrap
%option noinput nounput

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

"/*"([^*]|\*+[^*/])*\*+"/" {
    /* Ignorar comentários em bloco, contando as linhas que eles ocupam */
    int i;
    for (i = 0; i < yyleng; i++)
        if (yytext[i] == '\n') linha_atual++;
}
"#"[^\n]*       { /* Ignorar comentários de linha */ }

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.sval = strdup(yytext); /* Preenche o yylval.sval com a string copiada */    
    return ID; 
}

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

 /* Indentação: consome a quebra de linha, todas as linhas em branco ou
    somente-comentário que vierem em seguida, e a indentação da próxima
    linha real. Assim linhas vazias nunca afetam a pilha de indentação. */
\n([ \t]*(#[^\n]*)?\n)*[ \t]* {
    const char *ultima_quebra = strrchr(yytext, '\n');
    int espacos = (int)(yytext + yyleng - (ultima_quebra + 1));
    int i;

    for (i = 0; i < yyleng; i++)
        if (yytext[i] == '\n') linha_atual++;

    processar_indentacao(espacos);

    if (linha_tem_token) {
        linha_tem_token = 0;
        return NEWLINE;
    }
    if (!fila_vazia()) return token_da_fila();
    /* linha em branco no início do arquivo: nada a emitir */
}

[ \t]+      {  }

.               { fprintf(stderr, "Erro léxico: caractere inválido '%s' na linha %d\n", yytext, linha_atual); }

<<EOF>> {
    if (!fim_tratado) {
        int token = tratar_fim_de_arquivo();
        if (token) return token;
    }
    if (!fila_vazia()) return token_da_fila();
    yyterminate();
}

%%

/* Entrega ao parser os tokens de indentação pendentes antes de continuar a
   varredura, e marca quando a linha lógica atual tem conteúdo. */
int yylex(void) {
    int token;

    if (!fila_vazia()) return token_da_fila();

    token = scanner();

    if (token != 0 && token != NEWLINE && token != INDENT && token != DEDENT)
        linha_tem_token = 1;

    return token;
}
