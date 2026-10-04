 ; test_divzero.asm - Division by Zero Test
ldc 10
ldc 0
div        ; B = 10, A = 0 -> Should trigger runtime error
HALT