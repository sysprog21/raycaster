.section ".text.boot"

.global _start

_start:
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    teq r0, r1
    beq 2f
    strb r2, [r0], #1
    b 1b

2:  ldr sp, =_start

    bl mem_init
    bl uart_init

    bl kmain
3:
    wfe
    b 3b
