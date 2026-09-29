# TestesSemanticoErro2 - uso de variaveis nao declaradas

# Variavel nunca declarada
y = z + 1

# Variavel declarada em escopo interno nao vaza para fora
def funcao():
    interna = 5
    return interna

print(interna)
