; bubble.asm - Bubble Sort Implementation in IITPSx Assembly
; Sorts an array of numbers in ascending order in-place

    ldc array   ; Load base address of array
    stl 0       ; Local variable 0: array pointer
    ldc 5       ; Array length N = 5
    stl 1       ; Local variable 1: length N

outer_loop:
    ldl 1       ; Load N
    adc -1      ; N - 1
    brlz done   ; If N - 1 < 0, sorting is complete
    stl 1       ; Update N

    ldc 0       ; Initialize index i = 0
    stl 2       ; Local variable 2: index i

inner_loop:
    ldl 1       ; Load limit
    ldl 2       ; Load i
    sub         ; limit - i
    brlz outer_loop ; If i >= limit, restart outer loop

    ; Compute address of array[i]
    ldl 0       ; Base address
    ldl 2       ; Index i
    add         ; Base + i
    stl 3       ; Local variable 3: &array[i]

    ; Load array[i]
    ldl 3       ; &array[i]
    ldnl 0      ; A = array[i]
    stl 4       ; Local variable 4: array[i]

    ; Load array[i+1]
    ldl 3       ; &array[i]
    ldnl 1      ; A = array[i+1]
    stl 5       ; Local variable 5: array[i+1]

    ; Compare array[i] and array[i+1]
    ldl 4       ; array[i]
    ldl 5       ; array[i+1]
    sub         ; array[i] - array[i+1]
    brlz no_swap; If array[i] < array[i+1], no swap needed
    brz no_swap ; If array[i] == array[i+1], no swap needed

    ; Swap array[i] and array[i+1]
    ldl 3       ; &array[i]
    ldl 5       ; array[i+1]
    stnl 0      ; array[i] = array[i+1]

    ldl 3       ; &array[i]
    ldl 4       ; array[i]
    stnl 1      ; array[i+1] = array[i]

no_swap:
    ldl 2       ; Load i
    adc 1       ; i + 1
    stl 2       ; i = i + 1
    br inner_loop

done:
    HALT

; Unsorted input array
array:
    data 42
    data 15
    data 8
    data 99
    data 23
    