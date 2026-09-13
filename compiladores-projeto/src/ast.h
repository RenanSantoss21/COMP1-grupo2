#ifndef AST_H
#define AST_H

#include "tipos.h"

/* ── Tipos de nó da AST ──────────────────────────────────────────── */
typedef enum {
    NO_NUM_INT,
    NO_NUM_FLOAT,
    NO_STRING,
    NO_BOOL,
    NO_ID,
    NO_BINOP,
    NO_UNARYOP,
    NO_ASSIGN,
    NO_PRINT,
    NO_INPUT,
    NO_IF,
    NO_ELIF,
    NO_WHILE,
    NO_FOR,
    NO_FUNCDEF,
    NO_RETURN,
    NO_FUNCCALL,
    NO_BLOCO,
    NO_PROGRAMA
} TipoNo;

/* ── Operadores ──────────────────────────────────────────────────── */
typedef enum {
    OP_SOMA,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_GT,
    OP_LT,
    OP_GTE,
    OP_LTE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_NEG,
    OP_IN
} TipoOperador;

/* ── Nó da AST ───────────────────────────────────────────────────── */
typedef struct noAST {
    TipoNo tipo;

    /* Literais e identificadores */
    int    valor_int;
    double valor_float;
    char  *valor_string;
    char  *nome;

    /* Operador para BinOp e UnaryOp */
    int operador;

    /* Filhos principais */
    struct noAST *condicao;
    struct noAST *esquerda;
    struct noAST *direita;

    /* Blocos de código */
    struct noAST *bloco_if;
    struct noAST *bloco_else;

    /* Lista (elif, argumentos, parâmetros, comandos) */
    struct noAST **argumentos;
    int num_argumentos;

    /* Localização no código-fonte */
    int linha;
} NoAST;

/* ── Construtores ────────────────────────────────────────────────── */
NoAST *criar_no_int(int valor, int linha);
NoAST *criar_no_float(double valor, int linha);
NoAST *criar_no_string(char *valor, int linha);
NoAST *criar_no_bool(int valor, int linha);
NoAST *criar_no_id(char *nome, int linha);
NoAST *criar_no_binop(int op, NoAST *esq, NoAST *dir, int linha);
NoAST *criar_no_unaryop(int op, NoAST *operando, int linha);
NoAST *criar_no_assign(char *nome, NoAST *expr, int linha);
NoAST *criar_no_print(NoAST **args, int num_args, int linha);
NoAST *criar_no_input(NoAST *prompt, int linha);
NoAST *criar_no_if(NoAST *cond, NoAST *bloco, NoAST **elifs, int n_elifs,
                   NoAST *else_bloco, int linha);
NoAST *criar_no_elif(NoAST *cond, NoAST *bloco, int linha);
NoAST *criar_no_while(NoAST *cond, NoAST *bloco, int linha);
NoAST *criar_no_for(char *var, NoAST *range, NoAST *bloco, int linha);
NoAST *criar_no_funcdef(char *nome, NoAST **params, int n_params,
                        NoAST *bloco, int linha);
NoAST *criar_no_return(NoAST *expr, int linha);
NoAST *criar_no_funccall(char *nome, NoAST **args, int n_args, int linha);
NoAST *criar_no_bloco(NoAST **cmds, int n_cmds);
NoAST *criar_no_programa(NoAST **cmds, int n_cmds);

/* ── Utilitários ─────────────────────────────────────────────────── */
void imprimir_ast(NoAST *raiz, int nivel);
void liberar_ast(NoAST *raiz);

#endif
