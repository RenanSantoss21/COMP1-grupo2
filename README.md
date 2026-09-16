# Projeto de Compiladores com Flex e Bison

Este repositório contém um compilador em desenvolvimento utilizando as ferramentas **Flex** (para análise léxica) e **Bison** (para análise sintática). O projeto já conta com um lexer completo (com suporte a indentação, strings, números, operadores e 16 palavras-chave) e um parser funcional com validação sintática.

**Objetivo do Projeto:** Desenvolver um compilador que traduzirá código da linguagem **Python** para **C#**.

---

## 🛠️ Estrutura de Diretórios

O projeto principal está na pasta `compiladores-projeto/`:

- `compiladores-projeto/lexer/`: Contém os arquivos do analisador léxico (`lexer.l`).
- `compiladores-projeto/parser/`: Contém os arquivos do analisador sintático (`parser.y`).
- `compiladores-projeto/src/`: Reservado para arquivos fonte auxiliares em C.
- `compiladores-projeto/testes/`: Contém 5 arquivos de teste (`.py`) e um script de automação (`rodar_testes.sh`).
- `compiladores-projeto/docs/`: Arquivos Markdown da documentação do projeto (site gerado via MkDocs).
- `compiladores-projeto/Makefile`: Script para automação da compilação.

Demais pastas (como `semana 01` a `semana 06`) contêm exercícios práticos e materiais de estudo da disciplina.

--- 

## ⚙️ Como compilar e executar

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