# Geracao de C#: suite completa E2E (algoritmos, casos de borda, ADR-001 e escape @)

# 1. Expressoes dobraveis/propagaveis para validar --tac vs --opt
const_base = 2 + 3 * 4
const_dobro = const_base * 2
print("const:", const_base, const_dobro)

# 2. Fibonacci iterativo com while (primeiros 8 termos)
a_fib = 0
b_fib = 1
cont = 0
soma_fib = 0
while cont < 8:
    soma_fib = soma_fib + a_fib
    prox_fib = a_fib + b_fib
    a_fib = b_fib
    b_fib = prox_fib
    cont = cont + 1
print("fib_ultimo:", a_fib, "soma_fib:", soma_fib)

# 3. Aninhamento em 3 niveis (for -> while -> if/elif/else) com acumulador e booleano
acc3 = 0
achou_par = False
for lvl1 in range(3):
    lvl2 = 0
    while lvl2 < 3:
        if lvl1 == 0 and lvl2 == 0:
            acc3 = acc3 + 1
        elif lvl1 > lvl2:
            acc3 = acc3 + lvl1 + lvl2
            achou_par = True
        else:
            acc3 = acc3 + 2
        lvl2 = lvl2 + 1
print("aninhado3:", acc3, achou_par)

# 4. Lacos for de zero iteracoes
z = 0
for zi in range(0):
    z = z + 1
for zj in range(5, 2):
    z = z + 1
for zk in range(1, 5, -1):
    z = z + 1
print("zeros:", z)

# 5. Limites com expressoes compostas e passo negativo dinamico
a = 1
b = 4
soma_comp = 0
for idx_comp in range(a + 1, b * 2):
    soma_comp = soma_comp + idx_comp
lim = 7
step_neg = -2
for idx_neg in range(lim, 0, step_neg):
    soma_comp = soma_comp + idx_neg
print("composto:", soma_comp)

# 6. Comparacoes ordinais de string e bool em if/elif/while, divisao real e promocao ADR-001
s1 = "alfa"
s2 = "zeta"
if s1 > s2:
    ordem_str = "maior"
elif s1 < s2:
    ordem_str = "menor"
else:
    ordem_str = "igual"
print("ordem_str:", ordem_str)

b_cmp1 = True
b_cmp2 = False
cont_bool = 0
while b_cmp1 > b_cmp2:
    cont_bool = cont_bool + 1
    b_cmp1 = False
if b_cmp1 <= b_cmp2 and not (b_cmp1 > b_cmp2):
    cont_bool = cont_bool + 10
print("cont_bool:", cont_bool)

div_real = 7 / 2
print("div_real:", div_real)

cond_prom = False
if cond_prom:
    val_prom = 1.5
else:
    val_prom = 2
print("val_prom:", val_prom + 0.25)

if not cond_prom:
    val_prom_if = 1.5
else:
    val_prom_if = 2
print("val_prom_if:", val_prom_if + 0.25)

# 7. Hoisting + escape @ com multiplas palavras reservadas apenas do C#
if cond_prom:
    double = 10
    struct = 20
else:
    double = 30
    struct = 40
for namespace in range(3):
    double = double + namespace
    struct = struct + 1
print("reservadas:", double, struct)
