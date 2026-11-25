# Teste E2E 06: Programa completo da especificacao (docs/Especificacao_Linguagem.md)
def saudacao(nome):
    print("Olá")
    print(nome)

limite = 5
for i in range(limite):
    if i == 3:
        saudacao("Visitante")
    else:
        print(i)
