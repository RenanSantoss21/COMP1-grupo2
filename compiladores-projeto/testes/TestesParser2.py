# TestesParser2 - if / elif / else com blocos indentados

x = 10
y = 5

if x > y:
    print(x)

if x > y:
    print(x)
else:
    print(y)

if x == 1:
    print("um")
elif x == 2:
    print("dois")
elif x == 3:
    print("tres")
else:
    print("outro")

if x > 0 and y > 0:
    maior = x
    menor = y
    print(maior)
else:
    maior = y
    menor = x
    print(menor)

if not x < y:
    print("x nao e menor")

if x in [1, 2, 3]:
    print("pertence")
elif x in [10, 20]:
    print("pertence ao segundo")

# if dentro de if (else pertence ao if interno)
if x > 0:
    if y > 0:
        print("ambos positivos")
    else:
        print("so x positivo")

# if dentro de if (else pertence ao if externo)
if x > 0:
    if y > 0:
        print("ambos positivos")
else:
    print("x nao positivo")
