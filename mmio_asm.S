.global spin
spin:
    sub r0, #1
    teq r0, #0
    bne spin
    bx lr
