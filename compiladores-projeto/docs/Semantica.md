# Análise Semântica

A fase de Análise Semântica do compilador Python-C# é responsável por garantir que as construções sintáticas (que já foram validadas pelo Parser e transformadas em uma AST) façam sentido dentro das regras da linguagem.

Atualmente, o foco principal da nossa análise semântica engloba a **Verificação de Declaração de Variáveis**, o **Gerenciamento de Escopos**, a **Inferência e Verificação de Tipos** e a **Análise em Duas Passadas** para suporte a *forward references* de funções.

## Verificação de Variáveis

O nosso analisador percorre a Árvore Sintática Abstrata (AST) procurando por operações envolvendo variáveis:

- **Atribuição (`NO_ASSIGN`):** Quando uma atribuição é encontrada (ex: `x = 10`), o analisador semântico infere o tipo do valor e insere a variável "x" na [Tabela de Símbolos](TabelaSimbolos.md) utilizando a função `tabela_inserir`. Se a variável já existir no escopo atual, seu tipo é dinamicamente atualizado, permitindo que mude de tipo conforme a execução (ex: de int para string).
- **Uso de Variável (`NO_ID`):** Quando uma variável é lida (por exemplo, dentro de uma expressão ou em um `print(x)`), o analisador verifica se ela já foi declarada consultando a tabela de símbolos através de `tabela_buscar`. Se a variável não existir na tabela (nem no escopo atual, nem nos escopos pais), o compilador emite um erro semântico indicando a linha em que o uso irregular ocorreu:
  
  ```text
  Erro semântico na linha X: Variável 'Y' não declarada.
  ```

## Gerenciamento de Escopos

A verificação semântica adota a regra de escopos do Python:

- **Instruções de Controle de Fluxo (`if`, `while`, `for`):** Blocos identados (`NO_BLOCO`) e laços `for` (`NO_FOR`) *não* criam novos escopos. Variáveis atribuídas dentro dessas estruturas compartilham o escopo corrente e permanecem disponíveis após a saída do bloco.
- **Funções (`NO_FUNCDEF`):** Apenas definições de função criam um novo escopo (`tabela_entrar_escopo()`). Parâmetros formais e variáveis atribuídas no corpo da função residem no escopo local da função.
- **Isolamento de Funções** Ao término da análise do corpo da função, `tabela_sair_escopo()` é invocada, destruindo parâmetros e variáveis locais da tabela para que não vazem para o escopo chamador/externo.

## Inferência e Verificação de Tipos

Além de gerenciar variáveis, o módulo garante a integridade das operações e anota o tipo de cada nó na árvore sintática abstrata (usando o campo `tipo_resolvido` na `NoAST`), fundamental para a Geração de Código.

O motor conta com a função `inferir_tipo`, que percorre a árvore de baixo para cima resolvendo as seguintes regras:

- **Literais e Variáveis:** Identifica nativamente se um valor é inteiro, float, string ou booleano, e consulta a Tabela de Símbolos para resgatar o tipo de variáveis sendo utilizadas.
- **Operações Aritméticas (`+`, `-`, `*`, `/`):** Verifica se os operandos são numéricos, recusando operações com booleanos. Se a operação envolver um inteiro e um float, o analisador automaticamente **promove** o resultado da expressão para float (`cast` implícito). O operador `+` também suporta **concatenação de strings**: quando ambos os operandos são do tipo `TIPO_STRING`, o resultado é inferido como `TIPO_STRING`. Operações de subtração, multiplicação ou divisão com strings continuam sendo recusadas.
- **Divisão (`/`):** Em Python, `/` é sempre divisão real (`5 / 2 == 2.5`), então a divisão entre dois inteiros é inferida como `TIPO_FLOAT`. **Observação para a geração de código:** em C#, `5 / 2` resulta em `2` (divisão inteira), por isso o codegen deve emitir `(double)a / b` quando os dois operandos forem inteiros.
- **Operações Relacionais e Lógicas (`==`, `<`, `and`, etc.):** Comparações verificam a compatibilidade dos operandos (barrando comparação de string com número, por exemplo) e garantem que o nó da AST resultante sempre receba o `TIPO_BOOL`.
- **Condicionais Estritos (`if` e `while`):** O analisador obriga que a expressão dentro de um laço ou condição seja puramente booleana. Ao contrário de linguagens permissivas (que aceitariam `if "texto"`), nosso compilador exige `TIPO_BOOL` e lança um erro semântico direto se isso for violado.
- **Funções (Assinatura e Compatibilidade):** Chamadas de função (`NO_FUNCCALL`) não são apenas inferidas, mas passam por uma checagem rigorosa de contrato. O analisador valida se:
  1. O identificador corresponde a uma função existente.
  2. A quantidade de argumentos da chamada é idêntica à quantidade de parâmetros cadastrados na Tabela de Símbolos.
  3. Os tipos de todos os argumentos correspondem às especificações (caso conhecidas).
  
## Validação de Contexto de Execução

Alguns comandos da linguagem possuem restrições ligadas ao ambiente onde são criados.

- **Uso do comando `return`:**
  O analisador semântico controla a flag `dentro_de_funcao` da Tabela de Símbolos. Se um `NO_RETURN` for encontrado no nível global do arquivo (fora de uma definição), o analisador emite imediatamente: `Erro semântico... 'return' fora de função gera erro.`
- **Consistência do Tipo de Retorno:**
  Dentro de uma mesma função, se houver múltiplos nós `return` em caminhos de ramificação (ex: `if` / `else`), o analisador assegura que todos devolvam tipos compatíveis, rastreando o `tipo_retorno_atual` na tabela. Qualquer desvio gera um erro de inconsistência de tipo.

## Análise em Duas Passadas (Forward References)

Em Python, uma função pode chamar outra definida mais abaixo no arquivo, desde que a chamada só ocorra em tempo de execução (ex: dentro do corpo de outra função). Para suportar isso, a análise semântica executa **duas passagens** sobre os comandos de cada escopo:

1. **1ª passagem — `pre_registrar_funcoes`:** Percorre apenas os comandos do nível corrente (`NO_PROGRAMA` ou `NO_BLOCO`) e registra na tabela de símbolos o nome e o número de parâmetros de todos os nós `NO_FUNCDEF`, sem analisar seus corpos. Isso garante que toda função definida no módulo já seja conhecida antes da análise completa.
2. **2ª passagem — `analisar_semantica`:** Percorre normalmente todos os comandos, incluindo os corpos das funções. Agora, quando encontrar uma chamada a `g()` dentro de `f()`, o nome `g` já existe na tabela.

Esta estratégia também se aplica recursivamente ao **corpo de cada função**, permitindo funções aninhadas que se referenciam mutuamente.

**Comportamento decidido:** Chamar uma função *antes* de seu `def` no **nível do módulo** (execução imediata) continua sendo erro, pois em Python real isso também geraria um `NameError` em runtime.

```python
# Válido: a chamada de g() está dentro de f(), executada apenas quando f() for chamada.
def f():
    return g()
def g():
    return 1

# Também válido: recursão direta.
def f():
    return f()
```

## Testes

### Testes Unitários (TDD)

A implementação foi orientada a testes unitários (TDD). Para compilar e rodar os testes unitários semânticos:
```bash
make test_semantica
```
Esses testes simulam árvores AST básicas e atestam que os cenários de sucesso passam e variáveis com escopo incorreto (ou não declaradas) disparam erros precisamente contabilizados pela API.

### Testes de Integração

Além dos testes unitários, foram adicionados **6 testes de integração** que passam código Python real pelo compilador completo (Issue #17). Eles estão organizados na pasta `testes/` e são executados pelo script `rodar_testes.sh`.

**Casos válidos (devem passar sem erros):**

| Arquivo | Cenário Testado |
|---------|-----------------|
| `TestesSemanticaValidos1.py` | Atribuição e uso correto de variáveis |
| `TestesSemanticaValidos2.py` | Operações compatíveis e promoção int→float, concatenação de strings |
| `TestesSemanticaValidos3.py` | Chamada de função com quantidade correta de argumentos |

**Casos de erro (devem gerar erros semânticos):**

| Arquivo | Cenário Testado |
|---------|-----------------|
| `TestesSemanticaErros1.py` | Uso de variável não declarada |
| `TestesSemanticaErros2.py` | Operação com tipos incompatíveis (string + int) |
| `TestesSemanticaErros3.py` | Chamada de função com argumentos errados e `return` fora de função |

Para rodar todos os testes do compilador (incluindo semânticos):
```bash
./testes/rodar_testes.sh
```

## API do Módulo

O módulo semântico (`src/semantica.c` e `src/semantica.h`) tem como principal interface de comunicação a seguinte função:

```c
int analisar_semantica(NoAST *raiz, TabelaSimbolos *tabela);
```

- **Entrada:** O nó raiz da sub-árvore ou programa e um ponteiro para a `TabelaSimbolos`.
- **Saída (Retorno):** Um inteiro com a quantidade total de erros semânticos detectados. Se o retorno for `0`, significa que o programa é semanticamente válido neste escopo de verificação.

Internamente, utiliza também:

```c
TipoDado inferir_tipo(NoAST *raiz, TabelaSimbolos *tabela, int *erros);
```
Que avalia uma expressão, anota o nó da AST e soma na contagem caso encontre quebras de tipagem.
