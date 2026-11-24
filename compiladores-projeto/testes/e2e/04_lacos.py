# Teste E2E 04: Lacos (while e for com range)
contador = 5
fatorial = 1
while contador > 1:
    fatorial = fatorial * contador
    contador = contador - 1
print("fatorial 5:", fatorial)

soma1 = 0
for i in range(5):
    soma1 = soma1 + i
print("range(5):", soma1)

soma2 = 0
for i in range(2, 7):
    soma2 = soma2 + i
print("range(2, 7):", soma2)

soma_pares = 0
for i in range(0, 10, 2):
    soma_pares = soma_pares + i
print("range(0, 10, 2):", soma_pares)

soma_reg = 0
for i in range(6, 0, -2):
    soma_reg = soma_reg + i
print("range(6, 0, -2):", soma_reg)

passo = 3
soma_din = 0
for k in range(1, 10, passo):
    soma_din = soma_din + k
print("range dinamico:", soma_din)

grade = 0
for linha in range(3):
    for coluna in range(3):
        if linha == coluna:
            grade = grade + 1
print("diagonal:", grade)
