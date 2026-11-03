# ADR 001: Política de Tipagem Estática na Reatribuição de Variáveis

- **Status:** Aceito
- **Data:** 2026-11-02
- **Autor(es):** Grupo 2 (Compiladores 1)
- **Contexto da Decisão:** Issue #51 — *Semântica: Decidir e documentar a política de reatribuição com tipo diferente*

---

## 1. Contexto

A linguagem fonte deste compilador é um subconjunto de Python (linguagem dinamicamente tipada), enquanto a linguagem alvo da compilação é **C#** (linguagem fortemente e estaticamente tipada, conforme definido em [`CODING_STANDARDS.md`](../../CODING_STANDARDS.md)).

Em Python padrão, variáveis podem mudar livremente de tipo em tempo de execução:
```python
x = 10       # int
x = "texto"  # str
```

Entretanto, emitir código para C# exigiria que tais variáveis fossem mapeadas como `dynamic` ou `object`, adicionando custo de boxing/unboxing e runtime complexo, ou técnicas avançadas de *name mangling* (ex: `x_1`, `x_2`), o que aumentaria significativamente a complexidade da geração de código e das otimizações intermediárias (TAC).

---

## 2. Decisão

Optou-se por adotar uma **Política de Tipagem Estática por Primeira Inferência (Declaração Implícita Monomórfica)**:

1. **Declaração Implícita:**
   A primeira atribuição a uma variável (`x = expr`) infere e fixa o seu tipo primitivo na Tabela de Símbolos (`int`, `float`, `string` ou `bool`).

2. **Reatribuição Válida (Mesmo Tipo):**
   Reatribuições a uma variável já existente no escopo atual são permitidas desde que o valor atribuído seja do **mesmo tipo** previamente registrado.

3. **Promoção Numérica Permitida:**
   Se uma variável foi inferida/declarada como `float`, ela pode receber um valor do tipo `int` (promoção implícita para `float`), pois C# suporta nativamente a conversão implícita de `int` para `double`. O tipo da variável na Tabela de Símbolos permanece `float`.
   Uma variável declarada como `int`, contudo, **não** pode receber um `float`.

4. **Reatribuição Inválida (Tipos Incompatíveis):**
   Qualquer tentativa de reatribuir uma variável com um tipo incompatível no mesmo escopo (ex: `int` para `string`, `bool` para `int`, `int` para `float`) é considerada um **Erro Semântico** (`SEM-17`):
   ```text
   Erro semântico na linha <N>: Tipos incompatíveis na reatribuição da variável '<nome>'.
   ```

---

## 3. Consequências

- **Positivas:**
  - Facilita diretamente a geração de código intermediário (TAC) e a emissão de código C#, mapeando tipos estáticos nativos (`int`, `double`, `string`, `bool`).
  - Previne comportamentos ambíguos de tipagem durante otimizações de código (como *constant propagation* e *constant folding*).
  - Garante robustez semântica em tempo de compilação sem necessidade de runtime dinâmico.

- **Negativas / Limitações:**
  - O subconjunto de Python aceito pelo compilador restringe o dinamismo pleno de tipos em variáveis reutilizadas no mesmo escopo. Programas com reutilização polimórfica de nomes devem declarar variáveis com nomes distintos (ex: `novo_x = "texto"`).
