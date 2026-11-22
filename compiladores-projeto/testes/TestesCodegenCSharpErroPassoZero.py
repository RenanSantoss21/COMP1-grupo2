# Guarda de runtime para passo zero em range() (dinamico e constante)
passo_zero = 0
for i in range(0, 5, passo_zero):
    print(i)
for j in range(0, 5, 0):
    print(j)
