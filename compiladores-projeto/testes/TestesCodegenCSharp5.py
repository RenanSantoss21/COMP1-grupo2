# Geracao de C#: funcoes, return, chamadas e exemplo da especificacao (Issue #31)

def saudacao(nome):
    print("Olá")
    print(nome)

def soma(x, y):
    return x + y

def dobro(n):
    return n * 2

def maior(a, b):
    if a > b:
        res = a
        return res
    return b

limite = 5
for i in range(limite):
    if i == 3:
        saudacao("Visitante")
    else:
        print(i)

total = soma(3, 4)
print("total:", total)
print("dobro:", dobro(total))
print("maior:", maior(total, 5))
