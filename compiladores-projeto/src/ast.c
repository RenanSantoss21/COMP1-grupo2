#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Helper: aloca nó zerado ─────────────────────────────────────── */
static NoAST *alocar_no(TipoNo tipo, int linha) {
    NoAST *no = calloc(1, sizeof(NoAST));
    if (no == NULL) {
        fprintf(stderr, "Erro: falha ao alocar nó AST\n");
        exit(1);
    }
    no->tipo = tipo;
    no->linha = linha;
    no->tipo_resolvido = TIPO_DESCONHECIDO;
    return no;
}

/* ── Construtores ────────────────────────────────────────────────── */

NoAST *criar_no_int(int valor, int linha) {
    NoAST *no = alocar_no(NO_NUM_INT, linha);
    no->valor_int = valor;
    return no;
}

NoAST *criar_no_float(double valor, int linha) {
    NoAST *no = alocar_no(NO_NUM_FLOAT, linha);
    no->valor_float = valor;
    return no;
}

NoAST *criar_no_string(char *valor, int linha) {
    NoAST *no = alocar_no(NO_STRING, linha);
    no->valor_string = strdup(valor);
    return no;
}

NoAST *criar_no_bool(int valor, int linha) {
    NoAST *no = alocar_no(NO_BOOL, linha);
    no->valor_int = valor;
    return no;
}

NoAST *criar_no_id(char *nome, int linha) {
    NoAST *no = alocar_no(NO_ID, linha);
    no->nome = strdup(nome);
    return no;
}

NoAST *criar_no_binop(int op, NoAST *esq, NoAST *dir, int linha) {
    NoAST *no = alocar_no(NO_BINOP, linha);
    no->operador = op;
    no->esquerda = esq;
    no->direita = dir;
    return no;
}

NoAST *criar_no_unaryop(int op, NoAST *operando, int linha) {
    NoAST *no = alocar_no(NO_UNARYOP, linha);
    no->operador = op;
    no->esquerda = operando;
    return no;
}

NoAST *criar_no_assign(char *nome, NoAST *expr, int linha) {
    NoAST *no = alocar_no(NO_ASSIGN, linha);
    no->nome = strdup(nome);
    no->esquerda = expr;
    return no;
}

NoAST *criar_no_print(NoAST **args, int num_args, int linha) {
    NoAST *no = alocar_no(NO_PRINT, linha);
    no->argumentos = args;
    no->num_argumentos = num_args;
    return no;
}

NoAST *criar_no_input(NoAST *prompt, int linha) {
    NoAST *no = alocar_no(NO_INPUT, linha);
    no->esquerda = prompt;
    return no;
}

NoAST *criar_no_if(NoAST *cond, NoAST *bloco, NoAST **elifs, int n_elifs,
                   NoAST *else_bloco, int linha) {
    NoAST *no = alocar_no(NO_IF, linha);
    no->condicao = cond;
    no->bloco_if = bloco;
    no->argumentos = elifs;
    no->num_argumentos = n_elifs;
    no->bloco_else = else_bloco;
    return no;
}

NoAST *criar_no_elif(NoAST *cond, NoAST *bloco, int linha) {
    NoAST *no = alocar_no(NO_ELIF, linha);
    no->condicao = cond;
    no->bloco_if = bloco;
    return no;
}

NoAST *criar_no_while(NoAST *cond, NoAST *bloco, int linha) {
    NoAST *no = alocar_no(NO_WHILE, linha);
    no->condicao = cond;
    no->bloco_if = bloco;
    return no;
}

NoAST *criar_no_for(char *var, NoAST *range, NoAST *bloco, int linha) {
    NoAST *no = alocar_no(NO_FOR, linha);
    no->nome = strdup(var);
    no->esquerda = range;
    no->bloco_if = bloco;
    return no;
}

NoAST *criar_no_funcdef(char *nome, NoAST **params, int n_params,
                        NoAST *bloco, int linha) {
    NoAST *no = alocar_no(NO_FUNCDEF, linha);
    no->nome = strdup(nome);
    no->argumentos = params;
    no->num_argumentos = n_params;
    no->bloco_if = bloco;
    return no;
}

NoAST *criar_no_return(NoAST *expr, int linha) {
    NoAST *no = alocar_no(NO_RETURN, linha);
    no->esquerda = expr;
    return no;
}

NoAST *criar_no_funccall(char *nome, NoAST **args, int n_args, int linha) {
    NoAST *no = alocar_no(NO_FUNCCALL, linha);
    no->nome = strdup(nome);
    no->argumentos = args;
    no->num_argumentos = n_args;
    return no;
}

NoAST *criar_no_bloco(NoAST **cmds, int n_cmds) {
    NoAST *no = alocar_no(NO_BLOCO, 0);
    no->argumentos = cmds;
    no->num_argumentos = n_cmds;
    return no;
}

NoAST *criar_no_programa(NoAST **cmds, int n_cmds) {
    NoAST *no = alocar_no(NO_PROGRAMA, 0);
    no->argumentos = cmds;
    no->num_argumentos = n_cmds;
    return no;
}

/* ── Nomes legíveis para tipos de nó (debug) ─────────────────────── */
static const char *nome_tipo_no(TipoNo tipo) {
    switch (tipo) {
        case NO_NUM_INT:   return "NumInt";
        case NO_NUM_FLOAT: return "NumFloat";
        case NO_STRING:    return "String";
        case NO_BOOL:      return "Bool";
        case NO_ID:        return "Id";
        case NO_BINOP:     return "BinOp";
        case NO_UNARYOP:   return "UnaryOp";
        case NO_ASSIGN:    return "Assign";
        case NO_PRINT:     return "Print";
        case NO_INPUT:     return "Input";
        case NO_IF:        return "If";
        case NO_ELIF:      return "Elif";
        case NO_WHILE:     return "While";
        case NO_FOR:       return "For";
        case NO_FUNCDEF:   return "FuncDef";
        case NO_RETURN:    return "Return";
        case NO_FUNCCALL:  return "FuncCall";
        case NO_BLOCO:     return "Bloco";
        case NO_PROGRAMA:  return "Programa";
        default:           return "???";
    }
}

static const char *nome_operador(int op) {
    switch (op) {
        case OP_SOMA: return "+";
        case OP_SUB:  return "-";
        case OP_MULT: return "*";
        case OP_DIV:  return "/";
        case OP_EQ:   return "==";
        case OP_NEQ:  return "!=";
        case OP_GT:   return ">";
        case OP_LT:   return "<";
        case OP_GTE:  return ">=";
        case OP_LTE:  return "<=";
        case OP_AND:  return "and";
        case OP_OR:   return "or";
        case OP_NOT:  return "not";
        case OP_NEG:  return "-";
        case OP_IN:   return "in";
        default:      return "?";
    }
}

/* ── Indentação auxiliar ─────────────────────────────────────────── */
static void indentar(int nivel) {
    for (int i = 0; i < nivel; i++)
        printf("  ");
}

/* ── imprimir_ast ────────────────────────────────────────────────── */
void imprimir_ast(NoAST *raiz, int nivel) {
    if (raiz == NULL) return;

    indentar(nivel);

    switch (raiz->tipo) {
        case NO_NUM_INT:
            printf("NumInt: %d\n", raiz->valor_int);
            break;
        case NO_NUM_FLOAT:
            printf("NumFloat: %.6g\n", raiz->valor_float);
            break;
        case NO_STRING:
            printf("String: %s\n", raiz->valor_string);
            break;
        case NO_BOOL:
            printf("Bool: %s\n", raiz->valor_int ? "True" : "False");
            break;
        case NO_ID:
            printf("Id: %s\n", raiz->nome);
            break;
        case NO_BINOP:
            printf("BinOp: %s\n", nome_operador(raiz->operador));
            imprimir_ast(raiz->esquerda, nivel + 1);
            imprimir_ast(raiz->direita, nivel + 1);
            break;
        case NO_UNARYOP:
            printf("UnaryOp: %s\n", nome_operador(raiz->operador));
            imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_ASSIGN:
            printf("Assign: %s\n", raiz->nome);
            imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_PRINT:
            printf("Print\n");
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_INPUT:
            printf("Input\n");
            if (raiz->esquerda)
                imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_IF:
            printf("If\n");
            indentar(nivel + 1); printf("Cond:\n");
            imprimir_ast(raiz->condicao, nivel + 2);
            indentar(nivel + 1); printf("Entao:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            if (raiz->bloco_else) {
                indentar(nivel + 1); printf("Senao:\n");
                imprimir_ast(raiz->bloco_else, nivel + 2);
            }
            break;
        case NO_ELIF:
            printf("Elif\n");
            indentar(nivel + 1); printf("Cond:\n");
            imprimir_ast(raiz->condicao, nivel + 2);
            indentar(nivel + 1); printf("Entao:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_WHILE:
            printf("While\n");
            indentar(nivel + 1); printf("Cond:\n");
            imprimir_ast(raiz->condicao, nivel + 2);
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_FOR:
            printf("For: %s\n", raiz->nome);
            indentar(nivel + 1); printf("Range:\n");
            imprimir_ast(raiz->esquerda, nivel + 2);
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_FUNCDEF:
            printf("FuncDef: %s\n", raiz->nome);
            if (raiz->num_argumentos > 0) {
                indentar(nivel + 1); printf("Params:\n");
                for (int i = 0; i < raiz->num_argumentos; i++)
                    imprimir_ast(raiz->argumentos[i], nivel + 2);
            }
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_RETURN:
            printf("Return\n");
            if (raiz->esquerda)
                imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_FUNCCALL:
            printf("FuncCall: %s\n", raiz->nome);
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_BLOCO:
            printf("Bloco\n");
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_PROGRAMA:
            printf("Programa\n");
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
    }
}

/* ── liberar_ast ─────────────────────────────────────────────────── */
void liberar_ast(NoAST *raiz) {
    if (raiz == NULL) return;

    /* Libera filhos diretos */
    liberar_ast(raiz->condicao);
    liberar_ast(raiz->esquerda);
    liberar_ast(raiz->direita);
    liberar_ast(raiz->bloco_if);
    liberar_ast(raiz->bloco_else);

    /* Libera lista de argumentos */
    if (raiz->argumentos) {
        for (int i = 0; i < raiz->num_argumentos; i++)
            liberar_ast(raiz->argumentos[i]);
        free(raiz->argumentos);
    }

    /* Libera strings alocadas */
    free(raiz->valor_string);
    free(raiz->nome);

    free(raiz);
}
