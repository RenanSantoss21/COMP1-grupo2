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

## 🟡 Sprint 3: Árvore Sintática e Análise Semântica (Em Andamento)
Foco em representar o programa em memória e verificar regras de significado e coerência.
- **Árvore Sintática Abstrata (AST)**: Estruturas de nós (nodos) para representar o código em formato de árvore sem detalhes redundantes da sintaxe.
- **Tabela de Símbolos**: Estrutura para armazenar nomes de variáveis, seus tipos e seus escopos.
- **Verificação de Tipos**: Checagem inicial de semântica para garantir a coerência das operações matemáticas e atribuições.

## ⚪ Sprint 4: Geração de Código Alvo (A Fazer)
Foco em traduzir o modelo interno para a linguagem destino.
- **Geração de C#**: Percorrer a AST para emitir código válido em C#.
- **Tradução de Tipos e Bibliotecas**: Mapeamento das estruturas nativas em Python para equivalentes e *runtime* em C#.
- **Testes End-to-End**: Testar o código gerado, certificando que o comportamento executado no C# corresponda fielmente à versão original em Python.
