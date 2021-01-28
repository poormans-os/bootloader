//Allows the user to access the inb and outb function that are written in asm
#pragma once

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);