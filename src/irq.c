#include "stdio.h"
#include "idt.h"

typedef struct
{
    unsigned short int ip;
    unsigned short int cs;
    unsigned short int flags;
    unsigned short int sp;
    unsigned short int ss;
} interrupt_frame;

static _keyboardState keyboardState;

static const unsigned char scanset1[0xFF] = {
    /*pressed*/
    /*0-7*/ 0 /*null*/, 0 /*esc*/, '1', '2', '3', '4', '5', '6',
    /*8-15*/ '7', '8', '9', '0', '-', '=', 8 /*backspace*/, 0 /*tab*/,
    /*16-23*/ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    /*24-31*/ 'O', 'P', '[', ']', '\n' /*enter*/, 0 /*L ctrl*/, 'A', 'S',
    /*32-39*/ 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    /*40-47*/ '\'', '`', 0 /*L shift*/, '\\', 'Z', 'X', 'C', 'V',
    /*48-55*/ 'B', 'N', 'M', ',', '.', '/', 0 /*R shift*/, 0 /*keypad:* */,
    /*56-63*/ 0 /*L alt*/, ' ' /*space*/, 0 /*capslock*/, 0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/,
    /*64-71*/ 0 /*F6*/, 0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/, 0 /*numLock*/, 0 /*scrollLock*/, 0 /*keypad:7 */,
    /*72-79*/ 0 /*keypad:8 */, 0 /*keypad:9 */, 0 /*keypad:- */, 0 /*keypad:4 */, 0 /*keypad:5 */, 0 /*keypad:6 */, 0 /*keypad:+ */, 0 /*keypad:1 */,
    /*80-87*/ 0 /*keypad:2 */, 0 /*keypad:3 */, 0 /*keypad:0 */, 0 /*keypad:. */, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*F11*/,
    /*88-95*/ 0 /*F12*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*96-103*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*104-111*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*112-119*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*120-127*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*128-135*/ 0 /*null*/, 0 /*esc*/, '1', '2', '3', '4', '5', '6',
    /*136-143*/ '7', '8', '9', '0', '-', '=', 8 /*backspace*/, 0 /*tab*/,
    /*144-151*/ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    /*152-159*/ 'O', 'P', '[', ']', '\n' /*enter*/, 0 /*L ctrl*/, 'A', 'S',
    /*160-167*/ 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
    /*168-175*/ '\'', '`', 0 /*L shift*/, '\\', 'Z', 'X', 'C', 'V',
    /*176-183*/ 'B', 'N', 'M', ',', '.', '/', 0 /*R shift*/, 0 /*keypad:* */,
    /*184-191*/ 0 /*L alt*/, ' ' /*space*/, 0 /*capslock*/, 0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/,
    /*192-199*/ 0 /*F6*/, 0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/, 0 /*numLock*/, 0 /*scrollLock*/, 0 /*keypad:7 */,
    /*200-207*/ 0 /*keypad:8 */, 0 /*keypad:9 */, 0 /*keypad:- */, 0 /*keypad:4 */, 0 /*keypad:5 */, 0 /*keypad:6 */, 0 /*keypad:+ */, 0 /*keypad:1 */,
    /*208-215*/ 0 /*keypad:2 */, 0 /*keypad:3 */, 0 /*keypad:0 */, 0 /*keypad:. */, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*F11*/,
    /*216-223*/ 0 /*F12*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*224-231*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*232-239*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*240-247*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/,
    /*248-255*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/
};

static const unsigned char scanset2[0xFF] = {
    /*0-7*/ 0 /*null*/, 0 /*F9*/, 0 /*null*/, 0 /*F5*/, 0 /*F3*/, 0 /*F1*/, 0 /*F2*/, 0 /*F12*/,
    /*8-15*/ 0 /*null*/, 0 /*F10*/, 0 /*F8*/, 0 /*F6*/, 0 /*F4*/, '\t' /*tab*/, '`' /*`*/, 0 /*null*/,
    /*16-23*/ 0 /*null*/, 0 /*l-alt*/, 0 /*l-shift*/, 0 /*null*/, 0 /*l-ctrl*/, 'Q' /*Q*/, '1' /*1*/, 0 /*null*/,
    /*24-31*/ 0 /*null*/, 0 /*null*/, 'Z' /*Z*/, 'S' /*S*/, 'A' /*A*/, 'W' /*W*/, '2' /*2*/, 0 /*null*/,
    /*32-39*/ 0 /*null*/, 'C' /*C*/, 'X' /*X*/, 'D' /*D*/, 'E' /*E*/, '4' /*4*/, '3' /*3*/, 0 /*null*/,
    /*40-47*/ 0 /*null*/, ' ' /*space*/, 'V' /*V*/, 'F' /*F*/, 'T' /*T*/, 'R' /*R*/, '5' /*5*/, 0 /*null*/,
    /*48-55*/ 0 /*null*/, 'N' /*N*/, 'B' /*B*/, 'H' /*H*/, 'G' /*G*/, 'Y' /*Y*/, '6' /*6*/, 0 /*null*/,
    /*56-63*/ 0 /*null*/, 0 /*null*/, 'M' /*N*/, 'J' /*J*/, 'U' /*U*/, '7' /*7*/, '8' /*8*/, 0 /*null*/,
    /*64-71*/ 0 /*null*/, ',' /*,*/, 'K' /*K*/, 'I' /*I*/, 'O' /*O*/, '0' /*0*/, '9' /*9*/, 0 /*null*/,
    /*72-79*/ 0 /*null*/, '.' /*.*/, '/' /* / */, 'L' /*L*/, ';' /*;*/, 'P' /*P*/, '-' /*-*/, 0 /*null*/,
    /*80-87*/ 0 /*null*/, 0 /*null*/, '\'' /*'*/, 0 /*null*/, '[' /*[*/, '=' /*=*/, 0 /*null*/, 0 /*null*/,
    /*88-95*/ 0 /*CapsLock*/, 0 /*r-shift*/, 0 /*Enter*/, ']' /*]*/, 0 /*null*/, '\\' /*\*/, 0 /*null*/, 0 /*null*/,
    /*96-103*/ 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*null*/, 0 /*BackSpace*/, 0 /*null*/,
    /*104-111*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*112-119*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*120-127*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*128-135*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*136-143*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*144-151*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*152-159*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*160-167*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*168-175*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*176-183*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*184-191*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*192-199*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*200-207*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*208-215*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*216-223*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*224-231*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*232-239*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*240-247*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, 0 /*null*/,
    /*248-255*/ 0 /*null*/, 0, 0, 0, 0, 0, 0, /*null*/
};

__attribute__((interrupt)) void irq0(const interrupt_frame *frame)
{
    outb(0x20, 0x20);
}

__attribute__((interrupt)) void irq1(const interrupt_frame *_)
{
    unsigned int code = inb(0x60);

    if (code == 0xFA) // OK
    {
        keyboardState.lastCmd = 0;
        keyboardState.lastData = 0;
    }
    else if (code == 0xFE) //Resend
        outb(0x60, keyboardState.lastData);
    else if (code == 0xBA) // CapsLock Pressed
        keyboardState.capsLock = !keyboardState.capsLock;
    else if (code & 0x80)
    {
        code = scanset1[code];
        if (keyboardState.capsLock == 0 && (code >= 0x41 && code <= 0x5A))
            putchar(code + 0x20);
        else
            putchar(code);
    }

    outb(0x20, 0x20);
}