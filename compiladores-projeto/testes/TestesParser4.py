# TestesParser4 - Definicao e chamada de funcoes com parametros

def sem_parametros():
    return 1

def um_parametro(a):
    return a

def dois_parametros(a, b):
    return a + b

def tres_parametros(a, b, c):
    soma = a + b + c
    return soma

def sem_retorno(a):
    print(a)

def retorno_vazio(a):
    if a > 0:
        return
    print(a)

def corpo_com_varios_comandos(a, b):
    x = a + b
    y = a - b
    z = x * y
    print(z)
    return z

# Chamadas de funcao
sem_parametros()
um_parametro(1)
dois_parametros(1, 2)
tres_parametros(1, 2, 3)

r1 = sem_parametros()
r2 = um_parametro(10)
r3 = dois_parametros(r1, r2)
r4 = tres_parametros(1 + 1, 2 * 2, 3)

# Chamada com expressoes e chamadas aninhadas como argumento
r5 = dois_parametros(um_parametro(1), um_parametro(2))
r6 = um_parametro(dois_parametros(1, 2) + 3)

# Chamada dentro de estruturas de controle
if dois_parametros(1, 2) > 2:
    print(dois_parametros(1, 2))

while um_parametro(0) < 10:
    print(um_parametro(1))

for i in range(3):
    print(um_parametro(i))

# Funcao que chama outra funcao
def chama_outra(a, b):
    parcial = dois_parametros(a, b)
    return tres_parametros(parcial, a, b)

print(chama_outra(1, 2))
