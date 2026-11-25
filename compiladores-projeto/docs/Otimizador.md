# Otimizador (TAC e AST para CodeGen C#)

O módulo `src/otimizador.c` (com cabeçalho em `src/otimizador.h`) implementa as passadas de otimização tanto sobre o Código de Três Endereços (`otimizar_tac`, Issues **#27** e **#28**) quanto diretamente sobre a AST antes da geração de código C# (`otimizar_ast`, Issue **#61**).

## Uso

O compilador permite inspecionar o TAC ou emitir código C# já otimizado:

```bash
# Exibe o TAC original gerado a partir da AST:
./compilador --tac < programa.py

# Exibe o TAC otimizado após as passadas de otimização:
./compilador --opt < programa.py

# Otimiza a AST e gera código C# otimizado (sem misturar TAC no stdout):
./compilador --opt --cs < programa.py
./compilador --opt --cs saida.cs < programa.py

# Exibe o TAC otimizado e também gera o C# otimizado:
./compilador --tac --opt --cs saida.cs < programa.py
```

Também é possível executar a suíte de testes unitários específica do otimizador:

```bash
make test_otimizador
```

---

## Otimizações Implementadas

### 1. Constant Folding (Dobra de Constantes)
Avalia em tempo de compilação operações cujos operandos sejam constantes conhecidas, substituindo a instrução binária ou unária por uma atribuição direta (`TAC_ASSIGN`).

O folding opera recursivamente dentro de cada bloco básico: ao dobrar um resultado constante, seu valor é registrado na tabela local de constantes e propagado imediatamente para expressões subsequentes que o utilizam.

**Tipos suportados:**
- **Inteiros**: `+`, `-`, `*`, `/` (com proteção contra divisão por zero), e comparações relacionais (`==`, `!=`, `<`, `<=`, `>`, `>=`).
- **Ponto Flutuante**: operações aritméticas e relacionais quando ao menos um dos operandos é `float`.
- **Booleanos**: operadores lógicos `and` e `or`.
- **Textos**: concatenação com `+` entre literais de string (aspas simples ou duplas) e comparações (`==`, `!=`).
- **Unários**: negação aritmética (`-`) e lógica (`not`).

**Exemplo:**
```tac
t1 = 3 + 4   ->  t1 = 7
t2 = 10 * 2  ->  t2 = 20
t3 = t1 + t2 ->  t3 = 27
```

---

### 2. Simplificação Algébrica
Aplica identidades algébricas para reduzir expressões a formas mais simples e eficientes, sem necessidade de calcular o valor numérico exato:

| Expressão Original | Resultado Otimizado | Descrição |
|--------------------|---------------------|-----------|
| `x * 1` / `1 * x`  | `x`                 | Elemento neutro da multiplicação |
| `x + 0` / `0 + x`  | `x`                 | Elemento neutro da adição |
| `x * 0` / `0 * x`  | `0`                 | Elemento absorvente da multiplicação |
| `x - 0`            | `x`                 | Subtração de zero |
| `x / 1`            | `x`                 | Divisão por 1 |
| `x - x`            | `0`                 | Subtração de uma variável por si mesma |
| `x and True`       | `x`                 | Elemento neutro do operador `and` |
| `True and x`       | `x`                 | Comutativa do `and` com `True` |
| `x and False`      | `False`             | Elemento absorvente do operador `and` |
| `False and x`      | `False`             | Comutativa do `and` com `False` |
| `x or True`        | `True`              | Elemento absorvente do operador `or` |
| `True or x`        | `True`              | Comutativa do `or` com `True` |
| `x or False`       | `x`                 | Elemento neutro do operador `or` |
| `False or x`       | `x`                 | Comutativa do `or` com `False` |

---

### 3. Propagação de Constantes
Rastreia atribuições de constantes a variáveis ou temporários dentro do mesmo bloco básico. Sempre que a variável é lida antes de ser reatribuída, seu identificador é substituído pelo valor literal correspondente.

- **Fronteiras de Bloco Básico**: encontros com `TAC_LABEL`, `TAC_GOTO`, `TAC_FUNC_BEGIN` ou `TAC_FUNC_END` invalidam a tabela de constantes local, garantindo que hipóteses sobre constantes não vazem através de saltos ou laços de repetição.
- **Invalidação por Reatribuição**: se uma variável recebe novo valor (ou um valor não-constante), sua entrada anterior na tabela é invalidada.

---

### 4. Remoção de Código Morto (Dead Code Elimination)
Limpa instruções redundantes ou inalcançáveis:
1. **Eliminação de Temporários de Uso Único**: contrai cadeias como `t1 = a + b; x = t1` diretamente para `x = a + b`, removendo o temporário intermediário.
2. **Remoção de Temporários Não Utilizados**: instruções cujo resultado é um temporário (`t0`, `t1`, ...) que nunca é lido no restante do código são completamente removidas.
3. **Instruções Inalcançáveis**: elimina instruções posicionadas logo após `TAC_RETURN` ou `TAC_GOTO` incondicional até o próximo rótulo ou fim de função.

---

## Loop de Ponto Fixo

A função principal de otimização, `otimizar_tac(TAC *tac)`, executa as passadas em um laço iterativo até que nenhuma transformação adicional seja possível (ponto fixo) ou até atingir um limite máximo de segurança (20 iterações):

```c
while (modificou && iteracoes < 20) {
    modificou = false;
    if (otimizar_propagacao_constantes(tac)) modificou = true;
    if (otimizar_constant_folding(tac))      modificou = true;
    if (otimizar_remocao_codigo_morto(tac))  modificou = true;
    iteracoes++;
}
```

---

## Otimização sobre a AST (`otimizar_ast`)

Para a geração de código C# (`--opt --cs`), a função pública `int otimizar_ast(NoAST *raiz)` aplica as mesmas 4 técnicas diretamente sobre a AST antes de `codegen_csharp_gerar`, preservando a estrutura de alto nível (`if/elif/else`, `while`, `for` e métodos `static`) e liberando com `liberar_ast` os nós podados:

1. **Constant Folding na AST**: dobra `NO_BINOP` e `NO_UNARYOP` com operandos literais (`int`, `float`, `bool` e `string`), preservando `tipo_resolvido` e `linha`. Em Python 3, divisão entre dois inteiros (`5 / 2`) resulta em `NO_NUM_FLOAT` (`TIPO_FLOAT`), e divisões por zero são preservadas sem dobrar.
2. **Simplificação Algébrica Segura**: reduz identidades aritméticas e booleanas respeitando pureza (não descarta subexpressões com `NO_FUNCCALL`, `NO_INPUT` ou `NO_ASSIGN`) e tipos:
   - `x / 1` só é simplificado para `x` quando `x` já é `TIPO_FLOAT` (mantendo `(double)x / 1` quando `x` é `TIPO_INT`).
   - Operações com literal `float` (como `x + 0.0` com `x` inteiro) não perdem a promoção para `double`.
3. **Propagação de Constantes em Bloco Básico**: reutiliza `TabelaConstantes` para substituir `NO_ID` por literais conhecidos dentro do mesmo bloco sequencial (preservando `double` em variáveis promovidas pelo ADR-001), invalidando em reatribuições não-constantes e limpando a tabela em fronteiras de controle (`if/elif/else`, `while`, `for`) e isolando escopos de `def`.
4. **Remoção de Código Morto (DCE)**:
   - Elimina comandos após `NO_RETURN` no mesmo `NO_BLOCO`.
   - Remove `while False:` e `if False:` sem ramos alternativos.
   - Promove `elif` ou faz *splice* do `bloco_else` quando a condição do `if` reduz para `False`.
   - Faz *splice* do `bloco_if` diretamente no bloco pai quando a condição reduz para `True`.
