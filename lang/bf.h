#pragma once

#include "stdio.h"
#include "utils.h"

#define bf__TAPE_LEN 1024
#define MAX_CHAR_VALUE 255
#define END_OF_TAPE -1 //Reached end of BF tape.
#define UNDERFLOW -2   //A value on the tape is below 0.
#define OVERFLOW -3    //A value on the tape is above MAX_CHAR_VALUE.

typedef struct
{
    char *program;        //BrainF*ck program.
    int len;              //Length of BF program.
    char outBuffer[1024]; //Output of BF program.
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
    COMMA,         // ,
};

int bf__run();
int bf__main();

/*
BrainF*ck codes:
    ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.//Hello World!
    >>+>+>><<<<<++++++++++++[->>>[-]<[->>>+<<<]>>>[-<<+<+>>>]<<<<[->>>>+<<<<]>>>>[-<+>]<<<[->>>+<<<]>>>[-<<<+<+>>>>]<<<[-]>>><[->+<]>[-<+<<+>>>]<<<<<]>>>>. Fibonacci 89 (Y)
*/