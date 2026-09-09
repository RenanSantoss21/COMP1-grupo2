# TestesParser3 - while e for com blocos indentados

x = 10
total = 0

while x > 0:
    x = x - 1

while x < 10:
    x = x + 1
    total = total + x
    print(total)

while True:
    x = 0

while x > 0 and total < 100:
    total = total + x

for i in range(10):
    print(i)

for i in range(1, 10):
    total = total + i

for i in range(0, 20, 2):
    print(i)

for i in range(x + 1):
    print(i)

# while dentro de while
while x < 5:
    while total < 50:
        total = total + 1
    x = x + 1

# for dentro de for
for i in range(3):
    for j in range(3):
        print(i + j)

# for dentro de while
while x > 0:
    for i in range(x):
        print(i)
    x = x - 1

# if dentro de while
while x > 0:
    if x == 5:
        print("metade")
    else:
        print(x)
    x = x - 1
