# Analisador Léxico (Lexer)

O analisador léxico (ou Lexer) é responsável por ler o código-fonte como uma sequência de caracteres e convertê-lo em uma sequência de **tokens** significativos para o analisador sintático (Parser).

Esta página documenta os tokens e regras léxicas atualmente implementadas no projeto utilizando a ferramenta Flex.

## Tokens Implementados

### 1. Números
- O lexer reconhece sequências de dígitos como números inteiros.
- **Regex:** `[0-9]+`
- **Token Retornado:** `NUM` (salvando o valor inteiro em `yylval`).

### 2. Operadores Matemáticos
- **Soma (`+`):** Retorna `PLUS`
- **Subtração (`-`):** Retorna `MINUS`
- **Multiplicação (`*`):** Retorna `TIMES`
- **Divisão (`/`):** Retorna `DIVIDE`

### 3. Operadores Lógicos e Relacionais
Os seguintes operadores são utilizados para comparações e atribuições lógicas:
- **Igualdade (`==`):** Retorna `EQ`
- **Diferença (`!=`):** Retorna `NEQ`
- **Maior ou igual (`>=`):** Retorna `GTE`
- **Menor ou igual (`<=`):** Retorna `LTE`
- **Maior (`>`):** Retorna `GT`
- **Menor (`<`):** Retorna `LT`
- **Atribuição (`=`):** Retorna `ASSIGN`

### 4. Delimitadores
Símbolos utilizados para estruturação do código e agrupamento de expressões:
- **Parêntese Esquerdo (`(`):** Retorna `LPAREN`
- **Parêntese Direito (`)`):** Retorna `RPAREN`
- **Dois Pontos (`:`):** Retorna `COLON`
- **Vírgula (`,`):** Retorna `COMMA`

### 5. Comentários
Os comentários não geram tokens, eles são identificados e **descartados** (ignorados) pelo compilador, não afetando a análise sintática.

- **Comentários de linha:** Iniciam com o caractere `#` e vão até o final da linha.
  - Exemplo: `# Isto é um comentário de linha`
  - **Regex:** `"#"[^\n]*`

- **Comentários de bloco:** Textos delimitados por `/*` e `*/` podendo se estender por múltiplas linhas.
  - Exemplo: `/* Comentário de bloco */`
  - **Regex:** `"/*"([^*]|\*+[^*/])*\*+"/"`

### 6. Espaços em Branco
Espaços, tabulações e quebras de linha (`[ \t\n]+`) são simplesmente ignorados pelo lexer.

---

*Qualquer outro caractere não reconhecido pelas regras acima emitirá um aviso de "Caractere inválido".*
