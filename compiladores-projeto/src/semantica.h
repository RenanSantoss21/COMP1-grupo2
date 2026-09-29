#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"
#include "tabela.h"

/*
 * Percorre a AST verificando erros semânticos.
 * Retorna o número de erros encontrados (0 se não houver erro).
 */
int analisar_semantica(NoAST *raiz, TabelaSimbolos *tabela);

#endif
