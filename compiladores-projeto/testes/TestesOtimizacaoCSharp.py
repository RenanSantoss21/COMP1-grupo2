def resposta():
    if True:
        return 42
    morta = 99
    print("morto_apos_return")

def calc(n, f):
    r_add = n + 0
    r_mul0 = n * 0
    r_sub = n - n
    r_div_i = n / 1
    r_div_f = f / 2.0
    r_div_f1 = r_div_f / 1
    print(r_add, r_mul0, r_sub, r_div_i + 0.5, r_div_f1)
    return r_add + resposta()

x = 2 + 3 * 4
div_real = 5 / 2
msg = "comp" + "ilador"
cmp_txt = 'abc' == "abc"
print(x, div_real, msg, cmp_txt)

a = 5
b = a + 3
print(b)

flt = 1.5
flt = 2
g = flt
print(g)

i = 0
while i < 3:
    print(i)
    i = i + 1
print(i)

soma_for = 0
for k in range(3):
    soma_for = soma_for + k
print(soma_for)

while 1 > 2:
    print("morto_while")

if False:
    print("morto_if")
else:
    print("else_vivo")

res = calc(7, 3.5)
print(res)
