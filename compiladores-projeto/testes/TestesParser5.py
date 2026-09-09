# TestesParser5 - Estruturas aninhadas complexas

def classifica(lista, limite):
    acima = 0
    abaixo = 0
    for i in range(limite):
        if i > limite:
            acima = acima + 1
        elif i == limite:
            print("igual")
        else:
            abaixo = abaixo + 1
    return acima

def processa(n):
    total = 0
    i = 0
    while i < n:
        for j in range(i):
            if j > 0 and i > 0:
                if j == i:
                    total = total + 1
                else:
                    total = total + j
            else:
                total = total - 1
        i = i + 1
    return total

def tres_niveis(a):
    for i in range(a):
        while i > 0:
            if i == 1:
                print(i)
            i = i - 1
    return a

def com_if_elif_else_aninhado(x, y):
    if x > y:
        if x > 100:
            return 1
        elif x > 50:
            return 2
        else:
            return 3
    elif x == y:
        while x > 0:
            x = x - 1
        return 0
    else:
        for i in range(y):
            print(i)
        return y

resultado = classifica([1, 2, 3], 3)
if resultado > 0:
    for i in range(resultado):
        if i in [0, 1]:
            print(processa(i))
        else:
            print(tres_niveis(i))
else:
    print(com_if_elif_else_aninhado(1, 2))
