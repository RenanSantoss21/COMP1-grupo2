# Projeto de Compiladores com Flex e Bison

Este repositório contém a estrutura inicial de um projeto de compiladores utilizando as ferramentas Flex (para análise léxica) e Bison (para análise sintática). O projeto foi configurado de acordo com o guia da Semana 01.

**Objetivo do Projeto:** Desenvolver um compilador que traduzirá código da linguagem **Python** para **C#**.

---

## 🛠️ Estrutura de Diretórios

- `lexer/`: Contém os arquivos do analisador léxico (`lexer.l`).
- `parser/`: Contém os arquivos do analisador sintático (`parser.y`).
- `src/`: Contém arquivos fonte em C, como a função `main`.
- `Makefile`: Script para automação da compilação.

--- 

## ⚙️ Como compilar e executar

Para compilar o projeto, basta executar o comando:

```bash
make
```

Isso irá gerar um executável chamado `compilador` na raiz do projeto.

Para executá-lo:

```bash
./compilador
```

Após iniciar o programa, você pode digitar expressões como `3 + 4` e usar `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows) para finalizar a entrada.

### Limpeza

Para remover os arquivos gerados durante a compilação, execute:

```bash
make clean
```
---

## 👤 Integrantes & Funções

* **Arthur Amaral:** Palavras-chave e Identificadores. Responsável por "mapear" o vocabulário da linguagem.
* **Rafael Schetinger:** Símbolos, Operadores e Comentários. Responsável pela matemática e formatação do código.
* **Andris:** Literais. Responsável pelos dados que o usuário digita.
* **Renan Santos:** "Especialista" em Indentação.
* **Beatriz Brandão:** Testes (Quality Assurance - QA) e Makefile.
