# Projeto de Compiladores com Flex e Bison

Este repositório contém um compilador em desenvolvimento utilizando as ferramentas **Flex** (para análise léxica) e **Bison** (para análise sintática). O projeto conta com um lexer completo (com suporte a indentação, strings, números, operadores e 16 palavras-chave), um parser funcional com validação sintática e error recovery, uma Árvore Sintática Abstrata (AST), Tabela de Símbolos e Análise Semântica com verificação de tipos, escopos e funções.

**Objetivo do Projeto:** Desenvolver um compilador que traduzirá código da linguagem **Python** para **C#**.

---

## 🛠️ Estrutura de Diretórios

O projeto principal está na pasta `compiladores-projeto/`:

- `compiladores-projeto/lexer/`: Contém os arquivos do analisador léxico (`lexer.l`).
- `compiladores-projeto/parser/`: Contém os arquivos do analisador sintático (`parser.y`).
- `compiladores-projeto/src/`: Arquivos fonte auxiliares em C (AST, Tabela de Símbolos, Análise Semântica).
- `compiladores-projeto/testes/`: Contém testes léxicos, sintáticos e semânticos (`.py`) e um script de automação (`rodar_testes.sh`).
- `compiladores-projeto/testes/tdd/`: Testes unitários em C para AST, Tabela de Símbolos e Semântica.
- `compiladores-projeto/docs/`: Arquivos Markdown da documentação do projeto (site gerado via MkDocs).
- `compiladores-projeto/Makefile`: Script para automação da compilação.


Demais pastas (como `semana 01` a `semana 06`) contêm exercícios práticos e materiais de estudo da disciplina.

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

Após iniciar o programa, você pode digitar código Python válido. Exemplo:

```python
x = 10
if x > 5:
    print(x)
```

Use `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows) para finalizar a entrada.

### Rodando os Testes

O projeto inclui testes automatizados para o lexer, parser e análise semântica:

```bash
./testes/rodar_testes.sh
```

Para rodar testes unitários específicos de cada módulo:

```bash
make test_ast         # Testes unitários da AST
make test_tabela      # Testes unitários da Tabela de Símbolos
make test_semantica   # Testes unitários da Análise Semântica
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