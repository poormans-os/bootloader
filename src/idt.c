#include "idt.h"

//Each interrupt has this fields
typedef struct
{
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char ist;
    unsigned char type_attr;
    unsigned short int offset_middlebits;
    unsigned int offset_higherbits;
    unsigned int zero;
} IDT_entry;

IDT_entry IDT[256]; //All the interputs are stored in is table

extern _keyboardState keyboardState; //The keyboard state, defined at "irq.c"

extern void irq0(void *);
extern void irq1(void *);

inline void load_idt(unsigned long long *addr)
{
    __asm {
        lidt [addr]
        sti
    }
}

//initializing the IDT
void idt_init(void)
{
    // extern int load_idt(void *);
    unsigned long long irq0_address;
    unsigned long long irq1_address;

    unsigned long idt_address;
    unsigned long long idt_ptr[2];

    /* remapping the PIC */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 40);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    int offset = 64;

    irq0_address = (unsigned long long)irq0;
    IDT[offset].offset_lowerbits = irq0_address & 0xFFFF;
    IDT[offset].offset_middlebits = (irq0_address >> 16) & 0xFFFF;
    IDT[offset].offset_higherbits = (irq0_address >> 32) & 0xFFFFFFFF;
    IDT[offset].selector = 0x08;
    IDT[offset].type_attr = 0x8e;
    IDT[offset].ist = 0;
    IDT[offset].zero = 0;

    irq1_address = (unsigned long long)irq1;
    IDT[offset + 1].offset_lowerbits = irq1_address & 0xFFFF;
    IDT[offset + 1].offset_middlebits = (irq1_address >> 16) & 0xFFFF;
    IDT[offset + 1].offset_higherbits = (irq1_address >> 32) & 0xFFFFFFFF;
    IDT[offset + 1].selector = 0x08;
    IDT[offset + 1].type_attr = 0x8e;
    IDT[offset + 1].ist = 0;
    IDT[offset + 1].zero = 0;

    // IDT[0].offset_lowerbits = irq0_address & 0xffff;
    // IDT[0].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    // IDT[0].zero = 0;
    // IDT[0].type_attr = 0x8e; /* INTERRUPT_GATE */
    // IDT[0].offset_higherbits = (irq0_address & 0xffff0000) >> 16;

    // irq1_address = (unsigned long)irq1;
    // IDT[33].offset_lowerbits = irq1_address & 0xffff;
    // IDT[33].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
    // IDT[33].zero = 0;
    // IDT[33].type_attr = 0x8e; /* INTERRUPT_GATE */
    // IDT[33].offset_higherbits = (irq1_address & 0xffff0000) >> 16;

    /* fill the IDT descriptor */
    idt_address = (unsigned long)IDT;
    idt_ptr[0] = (sizeof(IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;

    load_idt(idt_ptr);

    // Keyboard init
    keyboardState.scanSet = 1;
    keyboardState.lastCmd = 0xF0;
    keyboardState.lastData = keyboardState.scanSet;
    outb(0x60, 0xF0);
    outb(0x60, keyboardState.scanSet);

    keyboardState.scrollLock = 0;
    keyboardState.numberLock = 1;
    keyboardState.capsLock = 0;

    outb(0x60, 0xED);
    outb(0x60, (keyboardState.capsLock << 2 & keyboardState.numberLock << 1 & keyboardState.scrollLock));
}