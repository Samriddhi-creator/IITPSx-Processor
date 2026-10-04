; test02.asm - Error Diagnostic Test
label:
label:         ; duplicate label definition
br nonesuch    ; no such label
ldc 08ge       ; not a number
ldc            ; missing operand
add 5          ; unexpected operand
bogus          ; bogus instruction