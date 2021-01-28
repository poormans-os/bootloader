#pragma once

#include "io.h"

typedef struct
{
    unsigned char scrollLock : 1;
    unsigned char numberLock : 1;
    unsigned char capsLock : 1;
    unsigned char scanSet : 2;
    unsigned char lastCmd;
    unsigned char lastData;
} __attribute__((packed)) _keyboardState;

static _keyboardState keyboardState;

void idt_init(void);