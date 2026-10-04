; test_mean.asm - Calculate integer mean of numbers
ldc 10
ldc 20
add
ldc 30
add
ldc 40
add        ; Sum = 100
ldc 4
div        ; Mean = 100 / 4 = 25
HALT