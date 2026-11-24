# Teste E2E 05: Funcoes (def, parametros, chamadas e return)
def cabecalho():
    print("=== Funcoes ===")

def somar(x, y):
    return x + y

def calcular_media(a, b):
    return (a + b) / 2

def classificar(valor):
    if valor > 0:
        return "positivo"
    elif valor < 0:
        return "negativo"
    return "zero"

def dobro_da_soma(x, y):
    total = somar(x, y)
    return total * 2

cabecalho()
r1 = somar(7, 8)
print("somar:", r1)
print("media:", calcular_media(5, 6))
print("classificar:", classificar(10), classificar(-3), classificar(0))
print("dobro_da_soma:", dobro_da_soma(3, 4))
