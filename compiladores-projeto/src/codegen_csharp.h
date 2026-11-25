#ifndef CODEGEN_CSHARP_H
#define CODEGEN_CSHARP_H

#include <stdio.h>
#include "ast.h"

/*
 * Gera o arquivo C# completo a partir da AST já validada pela semântica
 * (os tipos são lidos de tipo_resolvido).
 *
 * Estrutura emitida:
 *     using System;
 *
 *     namespace CompiladorPython {
 *         static class Program {
 *             (funções definidas pelo usuário)
 *
 *             static void Main(string[] args) {
 *                 (código do nível global)
 *             }
 *         }
 *     }
 *
 * Construções ainda não suportadas são emitidas como comentário, para que o
 * arquivo continue compilando. Retorna quantas foram encontradas
 * (0 = programa traduzido por completo).
 */
int codegen_csharp_gerar(NoAST *raiz, FILE *saida);

/*
 * Traduz uma expressão isolada para C#.
 * Retorna uma string alocada dinamicamente; o chamador deve liberá-la com free().
 */
char *codegen_csharp_expressao(NoAST *expr);

/*
 * Nome do tipo C# correspondente ao TipoDado.
 * TIPO_DESCONHECIDO vira "var" (o compilador C# infere a partir da expressão).
 */
const char *codegen_csharp_tipo(TipoDado tipo);

#endif
