#include "bf.h"

int bf__run(char *program)
{
    const int bufferLen = 1024;
    bf__data *bfmain = NULL;

    printf("HI bf__run\r\n");
    kmalloc(sizeof(bf__data), (void **)&bfmain);
    kmalloc(bufferLen, (void **)&bfmain->program);
    printf("HI bf__run\r\n");

    if (program != NULL)
    {
        free(bfmain->program);
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
    printf("HI bf__main\r\n");
    // printf("str: %s\r\n", data->program);
    // printf("str: %s\r\n", data->program);
    // printf("str: %s\r\n", data->program);

    while (prgPointer < len)
    {
        switch (program[prgPointer])
        {
        case '>':
            if (pointer < bf__TAPE_LEN)
                pointer++;
            // TODO else END_OF_TAPE
            break;
        case '<':
            if (pointer != 0)
                pointer--;
            // TODD else END_OF_TAPE
            break;
        case '+': // TODO overflow
            tape[pointer]++;
            break;
        case '-': // TODO underflow
            tape[pointer]--;
            break;
        case '.':
            putchar(tape[pointer]); //TODO
            outBuffer[bufferCounter] = tape[pointer];
            bufferCounter++;
            break;
        case ',':
            // tape[pointer] = getchar(); //TODO
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
