# TestesParserErro1 - Erros sintaticos (codigo invalido)
#
# Este arquivo reune os cinco erros exigidos pela issue. Com a recuperacao de
# erros o parser continua depois de cada erro e reporta todos eles numa unica
# execucao; os arquivos TestesParserErro2..5 isolam cada caso individualmente.

# 1) if sem ':'
if x > 1
    print(x)

# 2) def sem '():'
def soma
    return 1

# 3) for sem 'in range'
for i 10:
    print(i)

# 4) Indentacao inconsistente
if x > 1:
    y = 1
  z = 2

# 5) Parenteses nao balanceados
print(x
w = (1 + 2))
