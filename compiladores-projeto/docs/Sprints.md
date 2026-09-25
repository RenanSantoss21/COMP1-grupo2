# Planejamento de Sprints e Andamento do Projeto

O desenvolvimento deste compilador (Python para C#) está dividido em 4 sprints principais, seguindo as fases clássicas de compilação.

## 🟢 Sprint 1: Análise Léxica (Concluída)
Foco no reconhecimento de tokens, padrões e estrutura básica do código-fonte.
- **Lexer**: Implementado utilizando Flex.
- **Tokens Reconhecidos**: Palavras-chave (if, else, while, etc.), identificadores, literais (números, strings), operadores e símbolos de pontuação.
- **Indentação**: Lógica para lidar com os blocos baseados em recuo (espaços/tabs) no estilo Python (DEDENT e INDENT).
- **Testes**: Suíte inicial de testes léxicos criada.

## 🟢 Sprint 2: Análise Sintática (Concluída)
Foco em validar a estrutura das sentenças segundo a gramática da linguagem.
- **Parser**: Implementado utilizando Bison.
- **Validação Sintática**: Regras gramaticais implementadas para checar se a sequência de tokens forma expressões e blocos válidos de Python.
- **Recuperação de Erros**: Adição de tratativas para falhas de sintaxe e continuidade segura da leitura (quando possível).

## 🟢 Sprint 3: Árvore Sintática e Análise Semântica (Concluída)
Foco em representar o programa em memória e verificar regras de significado e coerência.
- **Árvore Sintática Abstrata (AST)**: Estruturas de nós (nodos) para representar o código em formato de árvore sem detalhes redundantes da sintaxe. (Issues #7, #8, #22)
- **Tabela de Símbolos**: Estrutura para armazenar nomes de variáveis, seus tipos e seus escopos. (Issue #9)
- **Verificação Semântica**: Verificação de variáveis declaradas, inferência e verificação de tipos, e validação de funções (parâmetros, retorno e chamadas). (Issues #10, #11, #12)
- **Testes Semânticos**: Suíte de testes de integração semântica (casos válidos e de erro) adicionada ao script de testes. (Issue #17)
- **Documentação**: AST e Tabela de Símbolos documentados no MkDocs. (Issue #19)
- **Limpeza**: Binários gerados removidos do rastreamento do Git, `.gitignore` atualizado. (Issue #32)

## 🟡 Sprint 4: Geração de Código Alvo (Em Andamento)
Foco em traduzir o modelo interno para a linguagem destino.
- **Código Intermediário (TAC)**: Criar estrutura base do código de três endereços e gerar TAC para expressões, atribuições, estruturas de controle e funções. (Issues #23, #24, #25, #26)
- **Otimizações**: Constant folding, simplificação algébrica, propagação de constantes e remoção de código morto. (Issues #27, #28)
- **Geração de C#**: Tradução de expressões, variáveis, estruturas de controle, funções, `print` e `input` para código C# executável. (Issues #29, #30, #31)
- **Testes End-to-End**: Testar o código gerado, certificando que o comportamento executado no C# corresponda fielmente à versão original em Python. (Issue #18)
- **CI/CD**: Integração de testes semânticos ao script, workflow de build automatizado e badges de status. (Issues #33, #34, #35, #36)
