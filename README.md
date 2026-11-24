[![CI](https://github.com/RenanSantoss21/COMP1-grupo2/actions/workflows/ci.yml/badge.svg)](https://github.com/RenanSantoss21/COMP1-grupo2/actions/workflows/ci.yml)
[![Deploy da documentação](https://github.com/RenanSantoss21/COMP1-grupo2/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/RenanSantoss21/COMP1-grupo2/actions/workflows/deploy-docs.yml)
# Projeto de Compiladores com Flex e Bison



Este repositório contém um compilador em desenvolvimento utilizando as ferramentas **Flex** (para análise léxica) e **Bison** (para análise sintática). O projeto conta com um lexer completo (com suporte a indentação, strings, números, operadores e 16 palavras-chave), um parser funcional com validação sintática e *error recovery*, Árvore Sintática Abstrata (AST), Tabela de Símbolos, Análise Semântica (verificação de tipos, escopos e funções), Geração de Código Intermediário (TAC), Otimizador TAC e Geração de Código C#.

**Objetivo do Projeto:** Desenvolver um compilador que traduzirá código da linguagem **Python** para **C#**.

---

## 🛠️ Estrutura de Diretórios

O projeto principal está na pasta `compiladores-projeto/`:

- `compiladores-projeto/lexer/`: Contém os arquivos do analisador léxico (`lexer.l`).
- `compiladores-projeto/parser/`: Contém os arquivos do analisador sintático (`parser.y`).
- `compiladores-projeto/src/`: Arquivos fonte auxiliares em C (AST, Tabela de Símbolos, Análise Semântica, Código Intermediário TAC, Otimizador e Gerador de Código C#).
- `compiladores-projeto/testes/`: Contém testes de integração (`.py`) léxicos, sintáticos, semânticos e de geração de código C#, além dos scripts de automação (`rodar_testes.sh` e `rodar_testes_e2e.sh`).
- `compiladores-projeto/testes/e2e/`: Programas de teste ponta a ponta (`.py`, `.in` e `.out`) que validam a tradução Mini-Python → C# compilado e executado.
- `compiladores-projeto/testes/tdd/`: Testes unitários em C para AST, Tabela de Símbolos, Semântica, TAC, Otimizador e Gerador de Código C#.
- `compiladores-projeto/docs/`: Arquivos Markdown da documentação do projeto (site gerado via MkDocs).
- `compiladores-projeto/Makefile`: Script para automação da compilação e dos testes.

A pasta `Aulas/` (`Aulas/semana 01` a `Aulas/semana 10`) contém exercícios práticos e materiais de estudo da disciplina.

--- 

## ⚙️ Como compilar e executar

### Pré-requisitos
É necessário ter o **Flex** e o **Bison** instalados no sistema, além das ferramentas básicas (`make` e `gcc`). 
Em distribuições Linux baseadas em Debian/Ubuntu, você pode instalar com:

```bash
sudo apt-get update
sudo apt-get install flex bison build-essential
```

### Compilação

Todas as operações principais devem ser feitas dentro do diretório do projeto:

```bash
cd compiladores-projeto
```

Para compilar o projeto, basta executar o comando:

```bash
make
```

Isso irá gerar um executável chamado `compilador` na raiz da pasta `compiladores-projeto`.

Para executá-lo:

```bash
./compilador
```

Após iniciar o programa interativamente, você pode digitar código Python válido. Exemplo:

```python
x = 10
if x > 5:
    print(x)
```

Use `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows) para finalizar a entrada.

### Flags de Linha de Comando

O executável `compilador` aceita as seguintes flags de inspeção e geração de código (podem ser combinadas):

| Flag | Descrição |
|---|---|
| `--ast` | Imprime a Árvore Sintática Abstrata (AST) indentada com os tipos resolvidos (`tipo_resolvido`). |
| `--dot` | Exporta a AST no formato Graphviz DOT para `stdout` e `ast.dot` (gera `ast.png` se `dot` estiver disponível). |
| `--tabela` | Exibe a Tabela de Símbolos a cada mudança de escopo durante a análise semântica. |
| `--tac` | Gera e imprime o Código Intermediário de Três Endereços (TAC). |
| `--opt` | Gera o TAC, aplica as passadas de otimização e imprime o TAC otimizado. |
| `--cs [arquivo]` | Gera o código-alvo em C# (grava em `arquivo` se informado; caso contrário, imprime no `stdout`). Só é gerado quando não há erros léxicos, sintáticos ou semânticos. |

Exemplos:

```bash
./compilador --ast --tabela < programa.py
./compilador --opt < programa.py
./compilador --cs saida.cs < programa.py
```

### Rodando os Testes

Para executar toda a suíte (testes unitários + testes de integração):

```bash
make test
```

Para rodar apenas os testes de integração (léxicos, sintáticos, semânticos e geração de código C#):

```bash
make test_integracao
# ou
./testes/rodar_testes.sh
```

Para rodar os testes de ponta a ponta (traduz Mini-Python para C#, compila com `dotnet`/`mcs`/`csc` e compara a saída com `.out` e `python3`):

```bash
make test_e2e
# ou
./testes/rodar_testes_e2e.sh
```

Para rodar testes unitários específicos de cada módulo:

```bash
make test_ast             # Testes unitários da AST
make test_tabela          # Testes unitários da Tabela de Símbolos
make test_semantica       # Testes unitários da Análise Semântica
make test_tac             # Testes unitários do Código Intermediário (TAC)
make test_otimizador      # Testes unitários do Otimizador TAC
make test_codegen_csharp  # Testes unitários do Gerador de Código C#
```

### Limpeza

Para remover os arquivos gerados durante a compilação, execute:

```bash
make clean
```
---

## 👤 Integrantes

* Arthur Amaral
* Rafael Schetinger
* Andris
* Renan Santos
* Beatriz Brandão