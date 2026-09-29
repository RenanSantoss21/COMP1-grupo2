# Coding Standards (Padrões de Código)

Este documento estabelece as regras e convenções que devem ser seguidas durante o desenvolvimento do compilador. Como nosso projeto tem duas vertentes (o compilador em si e o código que ele gera), dividimos os padrões em duas seções.

## 1. Padrões para o Código C (Implementação do Compilador)

Estas regras governam o código-fonte que constrói a AST, gerencia a Tabela de Símbolos, realiza a Semântica e gera o código alvo.

- **Nomenclatura por Módulo**: As funções devem ser prefixadas com o nome do módulo a que pertencem.
  - Exemplos: `tabela_inserir`, `tabela_buscar`, `criar_no_if`, `liberar_ast`.
  - _Motivação_: Em C, o namespace é global. Prefixos evitam colisões e deixam claro de onde vem a função.

- **Gerenciamento de Memória (Ownership)**: 
  - Para cada estrutura que possui um "construtor" (ex: `tabela_criar`, `criar_no_*`), deve haver um "destrutor" correspondente (`tabela_liberar`, `liberar_ast`).
  - O módulo que aloca a memória internamente deve fornecer a função para liberá-la. É proibido dar `free` em uma propriedade aninhada de fora do módulo (ex: nunca faça `free(simbolo->nome)` diretamente na semântica, delegue para a tabela).

- **Propagação de Erros**:
  - Os módulos de análise (Sintática e Semântica) não devem abortar o programa com `exit(1)` no primeiro erro. 
  - Eles devem imprimir o erro em `stderr` e retornar o acúmulo de erros para a main.
  - **Formato padrão de erro**: `Erro [tipo] na linha [X]: [Descrição clara].` (Ex: `Erro semântico na linha 15: Variável 'x' não declarada.`).

- **Encapsulamento**:
  - Módulos não devem expor a inicialização de seus campos internos diretamente em outros arquivos. (Por exemplo, `tabela.c` deve exportar métodos da API para inicializar Símbolos complexos).

## 2. Padrões para o Código C# (Geração de Código / Target)

Estas regras governam o "formato" do código C# que será emitido pela etapa de Geração de Código do compilador.

- **Mapeamento de Tipos Base**:
  - Python `int` -> C# `int`
  - Python `float` -> C# `double`
  - Python `str` -> C# `string`
  - Python `bool` -> C# `bool`

- **Estruturação do Programa (Boilerplate)**:
  - Todo o código gerado deve ser encapsulado dentro de um `namespace` e de uma classe `Program` estática.
  - O código global do Python (nível 0 de indentação que não seja definição de função) deve ser colocado dentro do método `static void Main(string[] args)`.

- **Mapeamento de Funções**:
  - Funções `def` do Python se tornam métodos `static` na classe `Program` do C#.

- **Tradução de Funções Built-in**:
  - `print(...)` deve ser mapeado exatamente para `Console.WriteLine(...)`.
  - `input(...)` deve ser mapeado exatamente para `Console.ReadLine()`. Caso tenha um prompt, deve gerar um `Console.Write(prompt)` antes do `ReadLine()`.

- **Mapeamento de Listas (Arrays)**:
  - Listas do Python (ex: `[1, 2, 3]`) devem preferencialmente ser traduzidas para `List<T>` do C# (ex: `new List<int> { 1, 2, 3 }`), exigindo `using System.Collections.Generic;` no topo do arquivo gerado.
