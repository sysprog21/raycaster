.section ".text.boot"

.global _start

_start:
    ldr sp, =_start
    bl kmain
1:
    wfe
    b 1b