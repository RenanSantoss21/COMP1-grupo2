#include "tabela.h"
#include <stdlib.h>
bool mostrar_tabela=false;
TabelaSimbolos* tabela_criar() {
    TabelaSimbolos* tab = malloc(sizeof(TabelaSimbolos));
    if (tab != NULL) {
        tab->escopo_atual = 0;
        tab->tamanho = 0;
        tab->capacidade = 10;
        tab->dentro_de_funcao = false;
        tab->tipo_retorno_atual = TIPO_DESCONHECIDO;
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
    s->pre_registrada = false;
    s->num_parametros = 0;
    s->tipos_parametros = NULL;
}

Simbolo* tabela_buscar(TabelaSimbolos* tab, char* nome) {
    for (int i = tab->tamanho - 1; i >= 0; i--) {
        if (strcmp(tab->array_simbolos[i].nome, nome) == 0) {
            return &tab->array_simbolos[i];
        }
    }
    return NULL;
}

Simbolo* tabela_buscar_escopo_atual(TabelaSimbolos* tab, char* nome) {
    if (tab == NULL || nome == NULL) return NULL;
    for (int i = tab->tamanho - 1; i >= 0; i--) {
        if (tab->array_simbolos[i].escopo == tab->escopo_atual &&
            strcmp(tab->array_simbolos[i].nome, nome) == 0) {
            return &tab->array_simbolos[i];
        }
    }
    return NULL;
}

void tabela_entrar_escopo(TabelaSimbolos* tab) {
    tab->escopo_atual++;
    if (mostrar_tabela) {
        tabela_imprimir(tab);
    }
}

void tabela_sair_escopo(TabelaSimbolos* tab) {
    if (mostrar_tabela) {
        tabela_imprimir(tab);
    }
    while (tab->tamanho > 0 && tab->array_simbolos[tab->tamanho - 1].escopo == tab->escopo_atual) {
        if (tab->array_simbolos[tab->tamanho - 1].tipos_parametros != NULL) {
            free(tab->array_simbolos[tab->tamanho - 1].tipos_parametros);
        }
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
        printf("Nome: %s, Tipo: %s, Escopo: %d, Linha: %d",
               s->nome, tipo_para_string(s->tipo), s->escopo, s->linha_declaracao);
        if(s->e_funcao){
            printf(", Parametros: (");
            for(int j=0;j<s->num_parametros;j++){
                if(j>0) printf(", ");
                printf("%s",tipo_para_string(s->tipos_parametros[j]));
            }
            printf(")");
        }
        printf("\n");
    }
    printf("---------------------------------------------\n");
}

void tabela_liberar(TabelaSimbolos* tab) {
    if (tab != NULL) {
        for (int i = 0; i < tab->tamanho; i++) {
            if (tab->array_simbolos[i].tipos_parametros != NULL) {
                free(tab->array_simbolos[i].tipos_parametros);
            }
            free(tab->array_simbolos[i].nome);
        }
        free(tab->array_simbolos);
        free(tab);
    }
}
