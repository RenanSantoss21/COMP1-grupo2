# TestesParserErro1 - Erros sintaticos (codigo invalido)
#
# Este arquivo reune os cinco erros exigidos pela issue. O Bison aborta no
# PRIMEIRO erro encontrado, entao os arquivos TestesParserErro2..5 isolam os
# demais casos para que cada um seja de fato exercitado pela suite.

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
