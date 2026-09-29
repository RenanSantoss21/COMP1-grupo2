# TestesSemantico1 - tipos, promocao numerica e condicoes booleanas validas

x = 10
y = 3.14
nome = "Beatriz"
ligado = True

# Aritmetica entre numeros (int + float promove para float)
soma = x + 5
media = (x + y) / 2
produto = x * y
negativo = -x

# Comparacoes entre tipos compativeis
maior = x > 5
mesmo_nome = nome == "Beatriz"
mesmo_estado = ligado == False

# Operadores logicos com booleanos
ambos = maior and ligado
algum = maior or mesmo_nome

# Condicoes estritamente booleanas
if x > 5:
    print("maior")
elif x == 5:
    print("igual")
else:
    print("menor")

while x > 0 and ligado:
    x = x - 1

if ligado:
    print(nome)

# Reatribuicao muda o tipo da variavel
x = "agora sou string"
print(x)
