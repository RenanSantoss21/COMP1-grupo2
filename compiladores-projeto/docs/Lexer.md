# Analisador Léxico (Lexer)

O analisador léxico (ou Lexer) é responsável por ler o código-fonte como uma sequência de caracteres e convertê-lo em uma sequência de **tokens** significativos para o analisador sintático (Parser).

Esta página documenta os tokens e regras léxicas atualmente implementadas no projeto utilizando a ferramenta Flex.

## Tokens Implementados

### 1. Palavras-Chave (Keywords)

O lexer reconhece as seguintes palavras reservadas da linguagem, cada uma mapeada para o seu token correspondente:

| Palavra-chave | Token retornado | Categoria |
|---|---|---|
| `if` | `IF` | Controle de fluxo |
| `elif` | `ELIF` | Controle de fluxo |
| `else` | `ELSE` | Controle de fluxo |
| `while` | `WHILE` | Laço de repetição |
| `for` | `FOR` | Laço de repetição |
| `in` | `IN` | Laço de repetição |
| `range` | `RANGE` | Função nativa |
| `def` | `DEF` | Definição de função |
| `return` | `RETURN` | Retorno de função |
| `print` | `PRINT` | Função nativa (saída) |
| `input` | `INPUT` | Função nativa (entrada) |
| `True` | `TRUE` | Literal booleano |
| `False` | `FALSE` | Literal booleano |
| `not` | `NOT` | Operador lógico |
| `and` | `AND` | Operador lógico |
| `or` | `OR` | Operador lógico |

> **Nota:** As palavras-chave são reconhecidas **antes** dos identificadores genéricos. Qualquer sequência alfanumérica que não corresponda a uma keyword é tratada como `ID`.

### 2. Identificadores

Nomes de variáveis e funções definidos pelo usuário.

- **Regex:** `[a-zA-Z_][a-zA-Z0-9_]*`
- **Token Retornado:** `ID`
- **Exemplos:** `soma`, `x`, `valor1`, `minha_funcao`

### 3. Literais Numéricos

O lexer distingue entre dois tipos de literais numéricos:

#### Números Inteiros
- **Regex:** `[0-9]+`
- **Token Retornado:** `NUM_INT` (valor armazenado em `yylval.ival`)
- **Exemplos:** `0`, `42`, `999999`

#### Números de Ponto Flutuante
- **Regex:** `[0-9]+"."[0-9]*([eE][+-]?[0-9]+)?` | `"."[0-9]+([eE][+-]?[0-9]+)?` | `[0-9]+[eE][+-]?[0-9]+`
- **Token Retornado:** `NUM_FLOAT` (valor armazenado em `yylval.dval`)
- **Exemplos:** `3.14`, `.5`, `2.0`, `1e10`, `3.14e2`, `.5e-3`

> **Nota:** A regra de ponto flutuante é posicionada antes da regra de inteiros no arquivo Flex, garantindo que números com ponto decimal ou notação científica sejam capturados corretamente.

### 4. Literais de String

Textos delimitados por aspas duplas ou simples, com suporte a sequências de escape.

- **Aspas duplas:** `\"([^\"\\]|\\.)*\"`
- **Aspas simples:** `'([^'\\]|\\.)*'`
- **Token Retornado:** `STRING_LITERAL` (valor armazenado em `yylval.sval`)
- **Exemplos:** `"Olá Mundo"`, `'hello world'`, `""`, `"linha\nnova"`

### 5. Operadores Matemáticos

| Operador | Token retornado |
|---|---|
| `+` | `PLUS` |
| `-` | `MINUS` |
| `*` | `TIMES` |
| `/` | `DIVIDE` |

### 6. Operadores Relacionais e de Atribuição

| Operador | Token retornado | Descrição |
|---|---|---|
| `==` | `EQ` | Igualdade |
| `!=` | `NEQ` | Diferença |
| `>=` | `GTE` | Maior ou igual |
| `<=` | `LTE` | Menor ou igual |
| `>` | `GT` | Maior que |
| `<` | `LT` | Menor que |
| `=` | `ASSIGN` | Atribuição |

> **Nota:** Os operadores de dois caracteres (`==`, `!=`, `>=`, `<=`) são posicionados antes dos de um caractere no Flex para evitar ambiguidade.

### 7. Delimitadores

Símbolos utilizados para estruturação do código e agrupamento de expressões:

| Símbolo | Token retornado | Descrição |
|---|---|---|
| `(` | `LPAREN` | Parêntese esquerdo |
| `)` | `RPAREN` | Parêntese direito |
| `[` | `LCOLCH` | Colchete esquerdo |
| `]` | `RCOLCH` | Colchete direito |
| `:` | `COLON` | Dois pontos |
| `,` | `COMMA` | Vírgula |

### 8. Comentários

Os comentários não geram tokens, eles são identificados e **descartados** (ignorados) pelo compilador, não afetando a análise sintática.

- **Comentários de linha:** Iniciam com o caractere `#` e vão até o final da linha.
  - Exemplo: `# Isto é um comentário de linha`
  - **Regex:** `"#"[^\n]*`

- **Comentários de bloco:** Textos delimitados por `/*` e `*/` podendo se estender por múltiplas linhas.
  - Exemplo: `/* Comentário de bloco */`
  - **Regex:** `"/*"([^*]|\*+[^*/])*\*+"/"`

### 9. Controle de Indentação (INDENT / DEDENT / NEWLINE)

Seguindo o modelo da linguagem Python, os blocos de código são definidos por níveis de indentação em vez de chaves `{}`. O lexer implementa um sistema baseado em pilha (_indent stack_) para rastrear os níveis de indentação e emitir os tokens apropriados.

#### Funcionamento

1. **Ao encontrar uma quebra de linha (`\n`)**, o lexer conta os espaços/tabulações que a seguem.
2. Se a quantidade de espaços **aumentou** em relação ao nível atual → emite `INDENT` e empilha o novo nível.
3. Se a quantidade de espaços **diminuiu** → emite `DEDENT` e desempilha. Se houver múltiplos níveis de recuo, o lexer re-injeta o texto com `yyless(0)` para emitir múltiplos `DEDENT` em sequência.
4. Se a quantidade **permaneceu igual** → emite `NEWLINE` (fim de instrução lógica).

#### Tratamento de Fim de Arquivo (EOF)

Ao atingir o fim do arquivo, o lexer emite tokens `DEDENT` pendentes para cada nível de indentação ainda aberto na pilha, garantindo que todos os blocos sejam fechados corretamente.

#### Exemplo

```python
if x > 0:        # NEWLINE
    print(x)      # INDENT + ... + NEWLINE
    if x > 10:    # ... + NEWLINE
        print(y)  # INDENT + ... + DEDENT + DEDENT (ao voltar)
```

### 10. Espaços em Branco

Espaços e tabulações **no meio de uma linha** (`[ \t]+`) são simplesmente ignorados pelo lexer. Quebras de linha (`\n`) **não são ignoradas**: elas disparam a lógica de indentação descrita acima.

---

*Qualquer outro caractere não reconhecido pelas regras acima emitirá um aviso de "Caractere inválido".*
