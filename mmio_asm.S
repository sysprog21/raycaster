.global delay
delay:
    sub r0, #1
    teq r0, #0
    bne delay
    bx lr
