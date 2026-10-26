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
//Função de debug: descomentar quando necessário para depuração da AST.
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

/* ── Impressão auxiliar de tipo resolvido ────────────────────────── */
static void imprimir_tipo_resolvido(TipoDado tipo) {
    if (tipo != TIPO_DESCONHECIDO) {
        printf(" [%s]", tipo_para_string(tipo));
    }
}

/* ── imprimir_ast ────────────────────────────────────────────────── */
void imprimir_ast(NoAST *raiz, int nivel) {
    if (raiz == NULL) return;

    indentar(nivel);

    switch (raiz->tipo) {
        case NO_NUM_INT:
            printf("%s: %d", nome_tipo_no(raiz->tipo), raiz->valor_int);
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            break;
        case NO_NUM_FLOAT:
            printf("%s: %.6g", nome_tipo_no(raiz->tipo), raiz->valor_float);
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            break;
        case NO_STRING:
            printf("%s: %s", nome_tipo_no(raiz->tipo), raiz->valor_string);
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            break;
        case NO_BOOL:
            printf("%s: %s", nome_tipo_no(raiz->tipo), raiz->valor_int ? "True" : "False");
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            break;
        case NO_ID:
            printf("%s: %s", nome_tipo_no(raiz->tipo), raiz->nome);
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            break;
        case NO_BINOP:
            printf("%s: %s", nome_tipo_no(raiz->tipo), nome_operador(raiz->operador));
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            imprimir_ast(raiz->esquerda, nivel + 1);
            imprimir_ast(raiz->direita, nivel + 1);
            break;
        case NO_UNARYOP:
            printf("%s: %s", nome_tipo_no(raiz->tipo), nome_operador(raiz->operador));
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_ASSIGN:
            printf("%s: %s\n", nome_tipo_no(raiz->tipo), raiz->nome);
            imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_PRINT:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_INPUT:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            if (raiz->esquerda)
                imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_IF:
            printf("%s\n", nome_tipo_no(raiz->tipo));
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
            printf("%s\n", nome_tipo_no(raiz->tipo));
            indentar(nivel + 1); printf("Cond:\n");
            imprimir_ast(raiz->condicao, nivel + 2);
            indentar(nivel + 1); printf("Entao:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_WHILE:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            indentar(nivel + 1); printf("Cond:\n");
            imprimir_ast(raiz->condicao, nivel + 2);
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_FOR:
            printf("%s: %s\n", nome_tipo_no(raiz->tipo), raiz->nome);
            indentar(nivel + 1); printf("Range:\n");
            imprimir_ast(raiz->esquerda, nivel + 2);
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_FUNCDEF:
            printf("%s: %s\n", nome_tipo_no(raiz->tipo), raiz->nome);
            if (raiz->num_argumentos > 0) {
                indentar(nivel + 1); printf("Params:\n");
                for (int i = 0; i < raiz->num_argumentos; i++)
                    imprimir_ast(raiz->argumentos[i], nivel + 2);
            }
            indentar(nivel + 1); printf("Corpo:\n");
            imprimir_ast(raiz->bloco_if, nivel + 2);
            break;
        case NO_RETURN:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            if (raiz->esquerda)
                imprimir_ast(raiz->esquerda, nivel + 1);
            break;
        case NO_FUNCCALL:
            printf("%s: %s", nome_tipo_no(raiz->tipo), raiz->nome);
            imprimir_tipo_resolvido(raiz->tipo_resolvido);
            printf("\n");
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_BLOCO:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
        case NO_PROGRAMA:
            printf("%s\n", nome_tipo_no(raiz->tipo));
            for (int i = 0; i < raiz->num_argumentos; i++)
                imprimir_ast(raiz->argumentos[i], nivel + 1);
            break;
    }
}

/* ── Exportação para Graphviz DOT ────────────────────────────────── */
static void escapar_string_dot(const char *str, FILE *saida) {
    if (!str) return;
    for (const char *p = str; *p != '\0'; p++) {
        if (*p == '"' || *p == '\\') {
            fputc('\\', saida);
        }
        fputc(*p, saida);
    }
}

static void imprimir_tipo_dot(TipoDado tipo, FILE *saida) {
    if (tipo != TIPO_DESCONHECIDO) {
        fprintf(saida, " [%s]", tipo_para_string(tipo));
    }
}

static int ast_para_dot_rec(NoAST *raiz, FILE *saida, int *contador) {
    if (raiz == NULL) return -1;

    int meu_id = (*contador)++;
    fprintf(saida, "    n%d [label=\"", meu_id);

    switch (raiz->tipo) {
        case NO_NUM_INT:
            fprintf(saida, "%s: %d", nome_tipo_no(raiz->tipo), raiz->valor_int);
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_NUM_FLOAT:
            fprintf(saida, "%s: %.6g", nome_tipo_no(raiz->tipo), raiz->valor_float);
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_STRING:
            fprintf(saida, "%s: ", nome_tipo_no(raiz->tipo));
            escapar_string_dot(raiz->valor_string, saida);
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_BOOL:
            fprintf(saida, "%s: %s", nome_tipo_no(raiz->tipo),
                    raiz->valor_int ? "True" : "False");
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_ID:
            fprintf(saida, "%s: ", nome_tipo_no(raiz->tipo));
            escapar_string_dot(raiz->nome, saida);
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_BINOP:
        case NO_UNARYOP:
            fprintf(saida, "%s: %s", nome_tipo_no(raiz->tipo),
                    nome_operador(raiz->operador));
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        case NO_ASSIGN:
        case NO_FOR:
        case NO_FUNCDEF:
            fprintf(saida, "%s: ", nome_tipo_no(raiz->tipo));
            escapar_string_dot(raiz->nome, saida);
            break;
        case NO_FUNCCALL:
            fprintf(saida, "%s: ", nome_tipo_no(raiz->tipo));
            escapar_string_dot(raiz->nome, saida);
            imprimir_tipo_dot(raiz->tipo_resolvido, saida);
            break;
        default:
            fprintf(saida, "%s", nome_tipo_no(raiz->tipo));
            break;
    }

    fprintf(saida, "\"];\n");

    switch (raiz->tipo) {
        case NO_BINOP: {
            int esq = ast_para_dot_rec(raiz->esquerda, saida, contador);
            int dir = ast_para_dot_rec(raiz->direita, saida, contador);
            if (esq >= 0) fprintf(saida, "    n%d -> n%d;\n", meu_id, esq);
            if (dir >= 0) fprintf(saida, "    n%d -> n%d;\n", meu_id, dir);
            break;
        }
        case NO_UNARYOP:
        case NO_ASSIGN:
        case NO_INPUT:
        case NO_RETURN: {
            int esq = ast_para_dot_rec(raiz->esquerda, saida, contador);
            if (esq >= 0) fprintf(saida, "    n%d -> n%d;\n", meu_id, esq);
            break;
        }
        case NO_IF: {
            int cond = ast_para_dot_rec(raiz->condicao, saida, contador);
            if (cond >= 0) fprintf(saida, "    n%d -> n%d [label=\"cond\"];\n", meu_id, cond);
            int entao = ast_para_dot_rec(raiz->bloco_if, saida, contador);
            if (entao >= 0) fprintf(saida, "    n%d -> n%d [label=\"entao\"];\n", meu_id, entao);
            for (int i = 0; i < raiz->num_argumentos; i++) {
                int elif_id = ast_para_dot_rec(raiz->argumentos[i], saida, contador);
                if (elif_id >= 0) fprintf(saida, "    n%d -> n%d [label=\"elif\"];\n", meu_id, elif_id);
            }
            if (raiz->bloco_else) {
                int senao = ast_para_dot_rec(raiz->bloco_else, saida, contador);
                if (senao >= 0) fprintf(saida, "    n%d -> n%d [label=\"senao\"];\n", meu_id, senao);
            }
            break;
        }
        case NO_ELIF: {
            int cond = ast_para_dot_rec(raiz->condicao, saida, contador);
            if (cond >= 0) fprintf(saida, "    n%d -> n%d [label=\"cond\"];\n", meu_id, cond);
            int entao = ast_para_dot_rec(raiz->bloco_if, saida, contador);
            if (entao >= 0) fprintf(saida, "    n%d -> n%d [label=\"entao\"];\n", meu_id, entao);
            break;
        }
        case NO_WHILE: {
            int cond = ast_para_dot_rec(raiz->condicao, saida, contador);
            if (cond >= 0) fprintf(saida, "    n%d -> n%d [label=\"cond\"];\n", meu_id, cond);
            int corpo = ast_para_dot_rec(raiz->bloco_if, saida, contador);
            if (corpo >= 0) fprintf(saida, "    n%d -> n%d [label=\"corpo\"];\n", meu_id, corpo);
            break;
        }
        case NO_FOR: {
            int range = ast_para_dot_rec(raiz->esquerda, saida, contador);
            if (range >= 0) fprintf(saida, "    n%d -> n%d [label=\"range\"];\n", meu_id, range);
            int corpo = ast_para_dot_rec(raiz->bloco_if, saida, contador);
            if (corpo >= 0) fprintf(saida, "    n%d -> n%d [label=\"corpo\"];\n", meu_id, corpo);
            break;
        }
        case NO_FUNCDEF: {
            for (int i = 0; i < raiz->num_argumentos; i++) {
                int param = ast_para_dot_rec(raiz->argumentos[i], saida, contador);
                if (param >= 0) fprintf(saida, "    n%d -> n%d [label=\"param\"];\n", meu_id, param);
            }
            int corpo = ast_para_dot_rec(raiz->bloco_if, saida, contador);
            if (corpo >= 0) fprintf(saida, "    n%d -> n%d [label=\"corpo\"];\n", meu_id, corpo);
            break;
        }
        case NO_PRINT:
        case NO_FUNCCALL:
        case NO_BLOCO:
        case NO_PROGRAMA: {
            for (int i = 0; i < raiz->num_argumentos; i++) {
                int arg = ast_para_dot_rec(raiz->argumentos[i], saida, contador);
                if (arg >= 0) fprintf(saida, "    n%d -> n%d;\n", meu_id, arg);
            }
            break;
        }
        default:
            break;
    }

    return meu_id;
}

void ast_para_dot(NoAST *raiz, FILE *saida) {
    if (saida == NULL) return;

    fprintf(saida, "digraph AST {\n");
    fprintf(saida, "    node [shape=box, style=\"rounded,filled\", fillcolor=\"#f8f9fa\", fontname=\"monospace\"];\n");
    fprintf(saida, "    edge [fontname=\"monospace\", fontsize=10];\n");

    int contador = 0;
    ast_para_dot_rec(raiz, saida, &contador);

    fprintf(saida, "}\n");
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
