#include <stdio.h>
#include <stdlib.h>
#include "semantica.h"

int analisar_semantica(NoAST *raiz, TabelaSimbolos *tabela) {
    if (raiz == NULL) return 0;
    
    int erros = 0;
    
    if (raiz->tipo == NO_ASSIGN) {
        // Insere a variável atribuída na tabela de símbolos
        tabela_inserir(tabela, raiz->nome, TIPO_DESCONHECIDO, raiz->linha);
        // Analisa a expressão à direita (que está armazenada em esquerda)
        if (raiz->esquerda) {
            erros += analisar_semantica(raiz->esquerda, tabela);
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
