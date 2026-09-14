# Teste semântico

# Inferência literal
x = 10
y = 3.14
z = "hello"

# Aritmética e Promoção
a = x + y # int + float -> float

# Aritmética com tipo incompatível (Erro esperado)
b = z + x # string + int -> Erro

# Comparação incompatível (Erro esperado)
if (x == z): # int == string -> Erro
    print("invalid")

# Condicional estrito (Erro esperado)
while ("texto"): # string como condição -> Erro
    print("loop")

def teste_retorno():
    return 10

# Chamada a função não declarada (Erro esperado)
w = funcao_inexistente()

c = teste_retorno()
