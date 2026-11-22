# Geracao de C#: estruturas de controle, hoisting de blocos e range (Issue #30)

# 1. if / elif / else com hoisting de variavel criada dentro de bloco
nota = 7
if nota >= 9:
    situacao = "excelente"
elif nota >= 5 and nota < 9:
    situacao = "aprovado"
else:
    situacao = "reprovado"
print("situacao:", situacao)

# Reatribuicao na raiz de variavel criada em bloco + palavra reservada C#
if nota > 0:
    nivel = 10
    string = 1
nivel = 20
print("nivel:", nivel, string)

# 2. while com acumulador e if aninhado
n = 4
soma_pares = 0
while n > 0:
    if n == 2 or n == 4:
        soma_pares = soma_pares + n
    n = n - 1
print("soma_pares:", soma_pares)

# 3. for range com 1 e 2 argumentos (e reuso do indice i)
acc = 0
for i in range(4):
    acc = acc + i
for i in range(2, 6):
    acc = acc + i
print("acc:", acc)

# 4. for range com 3 argumentos: passo constante positivo e negativo
soma_passo = 0
for j in range(0, 10, 2):
    soma_passo = soma_passo + j
for j in range(6, 0, -2):
    soma_passo = soma_passo + j
for j in range(3, 0, -1):
    soma_passo = soma_passo + j
print("soma_passo:", soma_passo)

# 5. for range com passo dinamico, colisao _passo0 e indice palavra reservada
_passo0 = 3
soma_din = 0
for k in range(0, 10, _passo0):
    soma_din = soma_din + k
passo_neg = -2
for k in range(5, -1, passo_neg):
    soma_din = soma_din + k
for switch in range(2):
    soma_din = soma_din + switch
p_din = 2
if True:
    for k in range(0, 4, p_din):
        soma_din = soma_din + k
_passo3 = 10
print("soma_din:", soma_din, _passo3)

# 6. Estruturas aninhadas: for dentro de for e if/elif/else dentro de for
matriz = 0
for a in range(3):
    for b in range(2):
        matriz = matriz + a * 10 + b
for c in range(3):
    if c == 0:
        tag = "zero"
    elif c == 1:
        tag = "um"
    else:
        tag = "dois"
    print(c, tag)
print("matriz:", matriz)
