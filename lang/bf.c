#include "bf.h"

int bf__run(char *program)
{
    const int bufferLen = 1024;
    bf__data *bfmain = NULL;

    kmalloc(sizeof(bf__data), (void **)&bfmain);
    kmalloc(bufferLen, (void **)&bfmain->program);
    // bfmain->program = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++."; //Hello World!
    //bfmain->program = ">>+>+>><<<<<++++++++++++[->>>[-]<[->>>+<<<]>>>[-<<+<+>>>]<<<<[->>>>+<<<<]>>>>[-<+>]<<<[->>>+<<<]>>>[-<<<+<+>>>>]<<<[-]>>><[->+<]>[-<+<<+>>>]<<<<<]>>>>."; //Fibbonacci 89 (Y)
    if (program != NULL)
    {
        bfmain->program = program;
    }
    else
    {
        printf("Please Enter Your Program:\r\n");
        bfmain->program = fgets(bfmain->program, bufferLen);
    }

    putchar('\r');
    putchar('\n');
    bfmain->len = strlen(bfmain->program);
    memset(bfmain->outBuffer, 0, 1024);
    printf("Running bf__main\r\n");
    return bf__main(bfmain);
    //addProcToQueue(printData, (void *)bfmain->outBuffer);
}

int bf__main(bf__data *data)
{
    char *program = data->program;
    int len = data->len;
    char *outBuffer = data->outBuffer;
    int bufferCounter = 0;

    unsigned int pointer = 0;
    unsigned int prgPointer = 0;
    unsigned int bracket = 0;
    short tape[bf__TAPE_LEN] = {0};

    while (prgPointer < len)
    {
        switch (program[prgPointer])
        {
        case '>':
            if (pointer < bf__TAPE_LEN)
                pointer++;
            else
                return END_OF_TAPE;
            break;
        case '<':
            if (pointer > 0)
                pointer--;
            else
                return END_OF_TAPE;
            break;
        case '+':
            if (tape[pointer] < MAX_CHAR_VALUE)
                tape[pointer]++;
            else
                return OVERFLOW;
            break;
        case '-':
            if (tape[pointer] > 0)
                tape[pointer]--;
            else
                return UNDERFLOW;
            break;
        case '.':
            putchar(tape[pointer]);
            outBuffer[bufferCounter] = tape[pointer];
            bufferCounter++;
            break;
        case ',':
            tape[pointer] = getchar();
            break;
        case '[':
            if (tape[pointer] == 0)
            {
                bracket = 0;
                prgPointer++;
                while (prgPointer < len)
                {
                    if (program[prgPointer] == ']' && bracket == 0)
                        break;
                    if (program[prgPointer] == '[')
                        bracket++;
                    else if (program[prgPointer] == ']')
                        bracket--;
                    prgPointer++;
                }
            }
            break;
        case ']':
            if (tape[pointer] != 0)
            {
                bracket = 0;
                prgPointer--;
                while (prgPointer >= 0)
                {
                    if (program[prgPointer] == '[' && bracket == 0)
                        break;
                    if (program[prgPointer] == ']')
                        bracket++;
                    else if (program[prgPointer] == '[')
                        bracket--;
                    prgPointer--;
                }
            }
            break;
        default:
            break;
        }
        prgPointer++;
    }
    return 0;
}
