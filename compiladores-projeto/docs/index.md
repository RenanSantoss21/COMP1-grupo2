# Bem-vindo à Documentação do Compilador

Este é o site de documentação oficial do nosso projeto da disciplina de Compiladores. O nosso objetivo final é construir um compilador funcional que traduz um subconjunto da linguagem **Python** para a linguagem **C#**.

Utilize o menu de navegação para explorar as etapas, requisitos e manuais de cada sprint.

---

## ⚙️ Como compilar e executar

### Pré-requisitos
É necessário ter o **Flex** e o **Bison** instalados, além de ferramentas básicas de compilação C (`make` e `gcc`). No Ubuntu/Debian, você pode instalar com:

```bash
sudo apt-get update
sudo apt-get install flex bison build-essential
```

### Compilação

Para compilar o projeto, basta executar o comando na pasta `compiladores-projeto`:

```bash
make
```

Isso irá gerar um executável chamado `compilador`. Para executá-lo:

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

O executável `compilador` aceita as seguintes flags (que podem ser combinadas):

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

---

## Objetivo do Semestre

Desenvolver as seguintes fases do compilador ao longo das semanas de aula:
1. **Análise Léxica (Flex)**: Tokenizar palavras-chave, variáveis, números e identações.
2. **Análise Sintática (Bison)**: Validar regras de estrutura (statements, expressions).
3. **Análise Semântica**: Tratar compatibilidade de tipos e escopo.
4. **Geração de Código Intermediário**: Representação abstrata (AST/Três endereços).
5. **Geração de Código Final**: Escrita do código em C# executável.

---

> *Este site é gerado automaticamente a partir dos arquivos Markdown localizados na pasta `docs/` através do MkDocs e do GitHub Actions.*
