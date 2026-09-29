# TestesSemanticoErro1 - erros semanticos variados (5 erros esperados)

x = 10
ligado = True

def soma(a, b):
    return a + b

# Operador logico com operando nao booleano
w = x and ligado

# Comparacao entre booleano e numero
comp = ligado == 1

# Numero errado de argumentos
r = soma(1)

# Chamada a algo que nao e funcao
q = x()

# return fora de funcao
return x
