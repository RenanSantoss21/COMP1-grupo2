# Projeto de Compiladores com Flex e Bison

Este repositório contém um compilador em desenvolvimento utilizando as ferramentas **Flex** (para análise léxica) e **Bison** (para análise sintática). O projeto já conta com um lexer completo (com suporte a indentação, strings, números, operadores e 16 palavras-chave) e um parser funcional com validação sintática.

**Objetivo do Projeto:** Desenvolver um compilador que traduzirá código da linguagem **Python** para **C#**.

---

## 🛠️ Estrutura de Diretórios

- `lexer/`: Contém os arquivos do analisador léxico (`lexer.l`).
- `parser/`: Contém os arquivos do analisador sintático (`parser.y`).
- `src/`: Reservado para arquivos fonte auxiliares em C.
- `testes/`: Contém 5 arquivos de teste (`.py`) e um script de automação (`rodar_testes.sh`).
- `docs/`: Arquivos Markdown da documentação do projeto (site gerado via MkDocs + GitHub Pages).
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

Após iniciar o programa, você pode digitar código Python válido. Exemplo:

```python
x = 10
if x > 5:
    print(x)
```

Use `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows) para finalizar a entrada.

### Rodando os Testes

O projeto inclui testes automatizados para o lexer:

```bash
./testes/rodar_testes.sh
```

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
