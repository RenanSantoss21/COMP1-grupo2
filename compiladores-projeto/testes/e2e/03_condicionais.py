# Teste E2E 03: Condicionais (if / elif / else)
nota = 8
if nota >= 9:
    conceito = "A"
elif nota >= 7 and nota < 9:
    conceito = "B"
elif nota >= 5:
    conceito = "C"
else:
    conceito = "D"
print("conceito:", conceito)

temperatura = 18
chovendo = False
if temperatura < 15 or chovendo:
    aviso = "levar casaco"
elif not chovendo and temperatura >= 15:
    aviso = "tempo agradavel"
else:
    aviso = "indefinido"
print("aviso:", aviso)

a = "alfa"
b = "beta"
if a < b:
    if nota != 0:
        print("ordem:", a, "<", b)
else:
    print("ordem inversa")
