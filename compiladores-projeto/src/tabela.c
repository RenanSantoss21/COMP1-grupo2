#include "tabela.h"
#include <stdlib.h>

TabelaSimbolos* tabela_criar() {
    TabelaSimbolos* tab = malloc(sizeof(TabelaSimbolos));
    if (tab != NULL) {
        tab->escopo_atual = 0;
        tab->tamanho = 0;
        tab->capacidade = 10;
        tab->array_simbolos = malloc(sizeof(Simbolo) * tab->capacidade);
    }
    return tab;
}

#include <string.h>

void tabela_inserir(TabelaSimbolos* tab, char* nome, TipoDado tipo, int linha) {
    if (tab->tamanho >= tab->capacidade) {
        tab->capacidade *= 2;
        tab->array_simbolos = realloc(tab->array_simbolos, sizeof(Simbolo) * tab->capacidade);
    }
    Simbolo* s = &tab->array_simbolos[tab->tamanho++];
    s->nome = strdup(nome);
    s->tipo = tipo;
    s->escopo = tab->escopo_atual;
    s->linha_declaracao = linha;
    s->e_funcao = false;
    s->num_parametros = 0;
}

Simbolo* tabela_buscar(TabelaSimbolos* tab, char* nome) {
    for (int i = tab->tamanho - 1; i >= 0; i--) {
        if (strcmp(tab->array_simbolos[i].nome, nome) == 0) {
            return &tab->array_simbolos[i];
        }
    }
    return NULL;
}

void tabela_entrar_escopo(TabelaSimbolos* tab) {
    tab->escopo_atual++;
}

void tabela_sair_escopo(TabelaSimbolos* tab) {
    while (tab->tamanho > 0 && tab->array_simbolos[tab->tamanho - 1].escopo == tab->escopo_atual) {
        free(tab->array_simbolos[tab->tamanho - 1].nome);
        tab->tamanho--;
    }
    tab->escopo_atual--;
}

#include <stdio.h>

void tabela_imprimir(TabelaSimbolos* tab) {
    printf("--- Tabela de Simbolos (Escopo Atual: %d) ---\n", tab->escopo_atual);
    for (int i = 0; i < tab->tamanho; i++) {
        Simbolo* s = &tab->array_simbolos[i];
        printf("Nome: %s, Tipo: %d, Escopo: %d, Linha: %d\n", s->nome, s->tipo, s->escopo, s->linha_declaracao);
    }
    printf("---------------------------------------------\n");
}

void tabela_liberar(TabelaSimbolos* tab) {
    if (tab != NULL) {
        for (int i = 0; i < tab->tamanho; i++) {
            free(tab->array_simbolos[i].nome);
        }
        free(tab->array_simbolos);
        free(tab);
    }
}
