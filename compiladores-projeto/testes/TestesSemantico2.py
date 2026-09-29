# TestesSemantico2 - funcoes, escopos e lacos validos

total = 0

def dobro(n):
    return n * 2

def saudacao():
    return "ola"

def maior(a, b):
    if a > b:
        return a
    return b

# Chamadas com a quantidade correta de argumentos
d = dobro(4)
s = saudacao()
m = maior(3, 7)

# Variavel global visivel dentro de blocos internos
for i in range(10):
    total = total + i

while total > 100:
    total = total - 1

# for aninhado usando a variavel de controle externa
for i in range(3):
    for j in range(3):
        print(i + j)

print(d, s, m, total)
