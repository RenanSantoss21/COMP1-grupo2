#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "codegen_intermediario.h"
#include <stdbool.h>

/* Verifica se uma string representa um valor constante conhecido (literal) */
bool e_constante(const char *val);

/* Passadas individuais de otimização sobre a estrutura TAC */
bool otimizar_propagacao_constantes(TAC *tac);
bool otimizar_constant_folding(TAC *tac);
bool otimizar_remocao_codigo_morto(TAC *tac);

/* Pipeline principal de otimização: executa as passadas em loop de ponto fixo */
int otimizar_tac(TAC *tac);

#endif
