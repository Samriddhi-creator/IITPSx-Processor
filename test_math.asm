; test_math.asm - Test arithmetic operations mul and div
ldc 12
ldc 5
mul        ; A = 12 * 5 = 60
ldc 4
div        ; A = 60 / 4 = 15
HALT