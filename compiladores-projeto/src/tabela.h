#ifndef TABELA_H
#define TABELA_H

#include "tipos.h"
#include <stdbool.h>

typedef struct {
    char* nome;
    TipoDado tipo;
    int escopo;
    int linha_declaracao;
    bool e_funcao;
    int num_parametros;
    TipoDado* tipos_parametros;
} Simbolo;

typedef struct {
    Simbolo* array_simbolos;
    int tamanho;
    int capacidade;
    int escopo_atual;
    bool dentro_de_funcao;
    TipoDado tipo_retorno_atual;
} TabelaSimbolos;

TabelaSimbolos* tabela_criar();
void tabela_inserir(TabelaSimbolos* tab, char* nome, TipoDado tipo, int linha);
Simbolo* tabela_buscar(TabelaSimbolos* tab, char* nome);
void tabela_entrar_escopo(TabelaSimbolos* tab);
void tabela_sair_escopo(TabelaSimbolos* tab);
void tabela_imprimir(TabelaSimbolos* tab);
void tabela_liberar(TabelaSimbolos* tab);

#endif
