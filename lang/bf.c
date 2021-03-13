#include "bf.h"

int bf__main(bf__data *data)
{
    printf("HW BF!\r\n");

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
            // TODO else END_OF_TAPE
            break;
        case '<':
            if (pointer != 0)
                pointer--;
            // TODD else END_OF_TAPE
        case '+': // TODO overflow
            tape[pointer]++;
            break;
        case '-': // TODO underflow
            tape[pointer]--;
            break;
        case '.':
            //putchar(tape[pointer]); //TODO
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
    }

    return 0;
}
