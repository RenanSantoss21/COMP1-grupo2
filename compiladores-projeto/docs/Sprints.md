# Planejamento de Sprints e Andamento do Projeto

O desenvolvimento deste compilador (Python para C#) está organizado em **5 sprints**, acompanhando a evolução das fases de compilação, estabilização da base e geração de código.

## 🟢 Sprint 1: Análise Léxica (Concluída)
Foco no reconhecimento de tokens, padrões e estrutura básica do código-fonte.
- **Lexer**: Implementado utilizando Flex (`lexer/lexer.l`).
- **Tokens Reconhecidos**: Palavras-chave (`if`, `else`, `while`, `for`, `def`, etc.), identificadores, literais (inteiros, floats, strings, booleanos), operadores e delimitadores.
- **Indentação**: Pilha de indentação e fila de tokens para emitir `NEWLINE`, `INDENT` e `DEDENT` no padrão Python.
- **Testes**: Suíte inicial de testes léxicos criada (`Testes1.py` a `Testes5.py`).

## 🟢 Sprint 2: Análise Sintática (Concluída)
Foco em validar a estrutura das sentenças segundo a gramática livre de contexto da linguagem.
- **Parser**: Implementado utilizando Bison (`parser/parser.y`), livre de conflitos shift/reduce.
- **Validação Sintática**: Regras gramaticais para expressões, atribuições, condicionais (`if`/`elif`/`else`), laços (`while` e `for ... in range(...)`) e funções (`def`/`return`).
- **Recuperação de Erros**: Sincronização com produções `error NEWLINE` e `error INDENT` para reportar múltiplos erros sintáticos em uma única execução.

## 🟢 Sprint 3: Árvore Sintática e Análise Semântica (Concluída)
Foco em representar o programa em memória e verificar regras iniciais de significado e coerência.
- **Árvore Sintática Abstrata (AST)**: Estruturas de nós (`NoAST`) e construtores para representar o programa de forma hierárquica. (Issues #7, #8, #22)
- **Tabela de Símbolos**: Estrutura em pilha contígua para armazenar variáveis, funções, parâmetros, tipos e escopos. (Issue #9)
- **Verificação Semântica**: Verificação de variáveis declaradas, inferência e promoção de tipos, e validação de funções. (Issues #10, #11, #12)
- **Testes e Limpeza**: Suíte inicial de testes semânticos, atualização do `Makefile` e remoção de binários rastreados pelo Git. (Issues #17, #20, #21, #32)

## 🟢 Sprint 4: Inspeção, Depuração, Refinamento Semântico e Documentação (Concluída)
Foco em conhecer profundamente o funcionamento interno do compilador, criar ferramentas visuais de inspeção, corrigir *edge cases* semânticos/sintáticos e consolidar a documentação técnica e a suíte de testes.
- **Ferramentas de Inspeção e Debug (CLI)**: Adição das flags `--ast` (impressão da AST anotada com `tipo_resolvido`), `--tabela` (inspeção da Tabela de Símbolos por escopo) e `--dot` (exportação visual da AST para Graphviz DOT e geração automática de `ast.png`). (Issues #40, #41)
- **Correção de Bugs Semânticos e Sintáticos (Bugfixes)**:
  - Ajuste das regras de escopo em blocos `if`/`while`/`for` para refletir o vazamento de escopo do Python, restringindo a criação de escopo a funções `def`. (Issue #45)
  - Inferência de tipos para operadores unários (`-`, `not`), `input()` (`TIPO_STRING`) e `in` (`TIPO_BOOL`). (Issue #47)
  - Ajuste da divisão (`/`) entre dois inteiros para resultar em `TIPO_FLOAT`, fiel à semântica do Python 3. (Issue #48)
  - Suporte a *forward references* e recursão mútua entre funções por meio de pré-registro em duas passadas (`pre_registrar_funcoes`). (Issue #49, #51)
  - Tratamento de reatribuição e redefinição no mesmo escopo, eliminação de erros duplicados de variáveis não declaradas e precisão de linhas na AST. (Issues #39, #46, #50, #52)
- **Testes Unitários (TDD) e CI/CD**: Ampliação dos testes unitários (`teste_tabela.c`, `teste_ast.c`, `teste_semantica.c`), integração completa no `rodar_testes.sh` (39 cenários), workflow de CI com `-Werror` e badges de status. (Issues #33, #34, #35, #36, #44)
- **Documentação Aprofundada**: Rastreio ponta a ponta em *Vida de um Programa no Compilador*, *Catálogo de Erros Semânticos* (`SEM-01` a `SEM-14`) e *Matriz de Campos da AST*. (Issues #19, #42, #43, #53)

## 🟡 Sprint 5: Geração de Código Intermediário, Otimização e Código Alvo C# (Em Andamento)
Foco em traduzir a AST validada para Código de Três Endereços (TAC), aplicar otimizações e emitir o código C# final.
- **Código Intermediário (TAC)**: Infraestrutura base (`codegen_intermediario.h` / `codegen_intermediario.c`) e geração de instruções TAC para expressões, atribuições, estruturas de controle (`if`/`elif`/`else`, `while`, `for`) e funções (`FUNC_BEGIN`/`FUNC_END`, `PARAM`, `CALL`, `RETURN`, `PRINT`). (Issues #13, #23, #24, #25, #26)
- **Otimizações de Código Intermediário**: *Constant folding*, simplificação algébrica, propagação de constantes e remoção de código morto. (Issues #15, #27, #28)
- **Geração de Código C#**: Tradução para uma classe estática `Program` com `Main`, mapeamento de tipos (`int`, `double`, `string`, `bool`), estruturas de controle, métodos estáticos e chamadas nativas (`Console.WriteLine`, `Console.ReadLine`). (Issues #14, #29, #30, #31)
- **Testes End-to-End**: Validação de programas completos garantindo que a saída do código C# compilado corresponda à execução no Python original. (Issue #18)
