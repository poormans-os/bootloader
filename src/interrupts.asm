[bits 64]; /*A simple wrapper-file for the IDT*/

global load_idt

load_idt:
    mov edx, [esp + 4]
    lidt [edx]
    sti
    ret
