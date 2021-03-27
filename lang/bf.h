#pragma once

#include "stdio.h"
#include "utils.h"

#define bf__TAPE_LEN 1024
#define MAX_CHAR_VALUE 255
#define END_OF_TAPE -1
#define UNDERFLOW -2
#define OVERFLOW -3

typedef struct
{
    char *program;
    int len;
    char outBuffer[1024];
} bf__data;

enum bf__tokens
{
    LEFT_ARROW,    // <
    RIGHT_ARROW,   // >
    PLUS_SIGN,     // +
    MINUS_SIGN,    // -
    OPEN_BRACKET,  // [
    CLOSE_BRACKET, // ]
    DOT,           // .
    COMMA,         // , TODO Implement input
};

int bf__run();
int bf__main();
