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

Após iniciar o programa, você pode digitar código Python válido. Exemplo:

```python
x = 10
if x > 5:
    print(x)
```

Use `Ctrl+D` (Linux/Mac) ou `Ctrl+Z` (Windows) para finalizar a entrada.

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
