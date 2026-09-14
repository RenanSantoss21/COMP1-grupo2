#include <stdio.h>
#include <stdlib.h>
#include "semantica.h"

TipoDado inferir_tipo(NoAST *raiz, TabelaSimbolos *tabela, int *erros) {
    if (raiz == NULL) return TIPO_DESCONHECIDO;

    TipoDado tipo_inferido = TIPO_DESCONHECIDO;

    switch (raiz->tipo) {
        case NO_NUM_INT:
            tipo_inferido = TIPO_INT;
            break;
        case NO_NUM_FLOAT:
            tipo_inferido = TIPO_FLOAT;
            break;
        case NO_STRING:
            tipo_inferido = TIPO_STRING;
            break;
        case NO_BOOL:
            tipo_inferido = TIPO_BOOL;
            break;
        case NO_ID: {
            Simbolo *sim = tabela_buscar(tabela, raiz->nome);
            if (sim != NULL) {
                tipo_inferido = sim->tipo;
            } else {
                fprintf(stderr, "Erro semântico na linha %d: Variável '%s' não declarada.\n", raiz->linha, raiz->nome);
                if (erros) *erros += 1;
            }
            break;
        }
        case NO_BINOP: {
            TipoDado tipo_esq = inferir_tipo(raiz->esquerda, tabela, erros);
            TipoDado tipo_dir = inferir_tipo(raiz->direita, tabela, erros);
            
            if (raiz->operador == OP_SOMA || raiz->operador == OP_SUB || 
                raiz->operador == OP_MULT || raiz->operador == OP_DIV) {
                
                if (tipo_esq == TIPO_STRING || tipo_dir == TIPO_STRING || 
                    tipo_esq == TIPO_BOOL || tipo_dir == TIPO_BOOL) {
                    fprintf(stderr, "Erro semântico na linha %d: Operação aritmética com tipos incompatíveis.\n", raiz->linha);
                    if (erros) *erros += 1;
                    tipo_inferido = TIPO_DESCONHECIDO;
                } else if (tipo_esq == TIPO_FLOAT || tipo_dir == TIPO_FLOAT) {
                    tipo_inferido = TIPO_FLOAT;
                } else if (tipo_esq == TIPO_INT && tipo_dir == TIPO_INT) {
                    tipo_inferido = TIPO_INT;
                } else {
                    tipo_inferido = TIPO_DESCONHECIDO;
                }
                
            } else if (raiz->operador == OP_EQ || raiz->operador == OP_NEQ ||
                       raiz->operador == OP_GT || raiz->operador == OP_LT ||
                       raiz->operador == OP_GTE || raiz->operador == OP_LTE) {
                
                if (tipo_esq == TIPO_STRING || tipo_dir == TIPO_STRING) {
                    if (tipo_esq != tipo_dir) {
                        fprintf(stderr, "Erro semântico na linha %d: Comparação inválida envolvendo string.\n", raiz->linha);
                        if (erros) *erros += 1;
                    }
                } else if (tipo_esq == TIPO_BOOL || tipo_dir == TIPO_BOOL) {
                    if (tipo_esq != tipo_dir) {
                        fprintf(stderr, "Erro semântico na linha %d: Comparação inválida envolvendo booleano.\n", raiz->linha);
                        if (erros) *erros += 1;
                    }
                }
                tipo_inferido = TIPO_BOOL;
                
            } else if (raiz->operador == OP_AND || raiz->operador == OP_OR) {
                if (tipo_esq != TIPO_BOOL || tipo_dir != TIPO_BOOL) {
                    fprintf(stderr, "Erro semântico na linha %d: Operador lógico exige booleanos.\n", raiz->linha);
                    if (erros) *erros += 1;
                }
                tipo_inferido = TIPO_BOOL;
            }
            break;
        }
        case NO_FUNCCALL: {
            Simbolo *sim = tabela_buscar(tabela, raiz->nome);
            if (sim != NULL) { // Simplificacao
                tipo_inferido = sim->tipo;
            } else {
                fprintf(stderr, "Erro semântico na linha %d: Chamada a função não declarada '%s'.\n", raiz->linha, raiz->nome);
                if (erros) *erros += 1;
                tipo_inferido = TIPO_DESCONHECIDO;
            }
            break;
        }
        default:
            break;
    }
    
    raiz->tipo_resolvido = tipo_inferido;
    return tipo_inferido;
}

int analisar_semantica(NoAST *raiz, TabelaSimbolos *tabela) {
    if (raiz == NULL) return 0;
    
    int erros = 0;
    
    if (raiz->tipo == NO_ASSIGN) {
        // Analisa a expressão à direita (que está armazenada em esquerda)
        if (raiz->esquerda) {
            erros += analisar_semantica(raiz->esquerda, tabela);
        }
        
        TipoDado tipo_dir = inferir_tipo(raiz->esquerda, tabela, &erros);
        
        // Verifica se a variável já existe no escopo atual. Se sim, atualiza o tipo (reatribuição dinâmica).
        Simbolo *sim = tabela_buscar(tabela, raiz->nome);
        if (sim != NULL && sim->escopo == tabela->escopo_atual) {
            sim->tipo = tipo_dir;
        } else {
            tabela_inserir(tabela, raiz->nome, tipo_dir, raiz->linha);
        }
        
        return erros;
    }
    
    if (raiz->tipo == NO_ID) {
        // Verifica se a variável já foi declarada
        Simbolo *sim = tabela_buscar(tabela, raiz->nome);
        if (sim == NULL) {
            fprintf(stderr, "Erro semântico na linha %d: Variável '%s' não declarada.\n", raiz->linha, raiz->nome);
            erros += 1;
        }
        return erros;
    }
    
    if (raiz->tipo == NO_BLOCO) {
        tabela_entrar_escopo(tabela);
        if (raiz->argumentos && raiz->num_argumentos > 0) {
            for (int i = 0; i < raiz->num_argumentos; i++) {
                erros += analisar_semantica(raiz->argumentos[i], tabela);
            }
        }
        tabela_sair_escopo(tabela);
        return erros;
    }
    
    if (raiz->tipo == NO_IF || raiz->tipo == NO_WHILE || raiz->tipo == NO_ELIF) {
        if (raiz->condicao) {
            TipoDado tipo_cond = inferir_tipo(raiz->condicao, tabela, &erros);
            if (tipo_cond != TIPO_BOOL && tipo_cond != TIPO_DESCONHECIDO) {
                fprintf(stderr, "Erro semântico na linha %d: A condição deve ser estritamente booleana.\n", raiz->linha);
                erros += 1;
            }
            erros += analisar_semantica(raiz->condicao, tabela);
        }
        if (raiz->bloco_if) erros += analisar_semantica(raiz->bloco_if, tabela);
        if (raiz->bloco_else) erros += analisar_semantica(raiz->bloco_else, tabela);
        if (raiz->argumentos && raiz->num_argumentos > 0) {
            for (int i = 0; i < raiz->num_argumentos; i++) {
                erros += analisar_semantica(raiz->argumentos[i], tabela);
            }
        }
        return erros;
    }
    
    if (raiz->tipo == NO_FUNCDEF) {
        Simbolo *s = tabela_buscar(tabela, raiz->nome);
        if (!s || s->escopo != tabela->escopo_atual) {
            tabela_inserir(tabela, raiz->nome, TIPO_DESCONHECIDO, raiz->linha);
            s = tabela_buscar(tabela, raiz->nome);
            s->e_funcao = true;
            s->num_parametros = raiz->num_argumentos;
        }
        
        tabela_entrar_escopo(tabela);
        if (raiz->argumentos) {
            for (int i = 0; i < raiz->num_argumentos; i++) {
                tabela_inserir(tabela, raiz->argumentos[i]->nome, TIPO_DESCONHECIDO, raiz->argumentos[i]->linha);
            }
        }
        erros += analisar_semantica(raiz->bloco_if, tabela);
        tabela_sair_escopo(tabela);
        return erros;
    }
    
    if (raiz->tipo == NO_RETURN) {
        if (raiz->esquerda) {
            inferir_tipo(raiz->esquerda, tabela, &erros);
            erros += analisar_semantica(raiz->esquerda, tabela);
        }
        return erros;
    }
    
    // Percurso recursivo básico da AST para os outros nós
    if (raiz->esquerda) erros += analisar_semantica(raiz->esquerda, tabela);
    if (raiz->direita) erros += analisar_semantica(raiz->direita, tabela);
    if (raiz->condicao) erros += analisar_semantica(raiz->condicao, tabela);
    if (raiz->bloco_if) erros += analisar_semantica(raiz->bloco_if, tabela);
    if (raiz->bloco_else) erros += analisar_semantica(raiz->bloco_else, tabela);
    
    if (raiz->argumentos && raiz->num_argumentos > 0) {
        for (int i = 0; i < raiz->num_argumentos; i++) {
            erros += analisar_semantica(raiz->argumentos[i], tabela);
        }
    }
    
    return erros;
}
