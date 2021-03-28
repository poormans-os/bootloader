#include "bf.h"

/**
 * @brief This function sets everything needed for executing a BF program, and calls for a function that will run it.
 * 
 * @param program A BrainF*ck program.
 * @return int Status of the BF program run.
 *          0 - Success.
 *         -1 - Reached end of BF tape.
 *         -2 - A value on the tape is below 0.
 *         -3 - A value on the tape is above MAX_CHAR_VALUE.
 */
int bf__run(char *program)
{
    const int bufferLen = 1024;
    bf__data *bfmain = NULL;
    int result = 0;

    //Allocating memory for the BF arguments.
    kmalloc(sizeof(bf__data), (void **)&bfmain);

    if (program != NULL && strlen(program) != 0) //There is content in the BF file.
    {
        printf("Running BrainF*ck\r\n");
        bfmain->program = program;
    }
    else //There is no content in the BF file - get a program from user input.
    {
        kmalloc(bufferLen, (void **)&bfmain->program);
        for (size_t i = 0; i < bufferLen; i++)
            bfmain->program[i] = 0;

        printf("Please Enter Your Program:\r\n");
        bfmain->program = fgets(bfmain->program, bufferLen);
    }

    bfmain->len = strlen(bfmain->program);
    memset(bfmain->outBuffer, 0, 1024);

    result = bf__main(bfmain); //Calling function that will execute the BF code.
    return result;
}

/**
 * @brief This function executes a BrainF*ck code.
 * 
 * @param data Arguments - BF Program, Len, Output.
 * @return int Status of the BF program run.
 *          0 - Success.
 *         -1 - Reached end of BF tape.
 *         -2 - A value on the tape is below 0.
 *         -3 - A value on the tape is above MAX_CHAR_VALUE.
 */
int bf__main(bf__data *data)
{
    char *program = data->program;
    int len = data->len;
    char *outBuffer = data->outBuffer;
    int bufferCounter = 0;

    //Initializing variables necessary for the BF program.
    unsigned int pointer = 0;
    unsigned int prgPointer = 0;
    unsigned int bracket = 0;
    short tape[bf__TAPE_LEN] = {0};

    while (prgPointer < len)
    {
        switch (program[prgPointer])
        {
        case '>': //Move to the next index on the tape.
            if (pointer < bf__TAPE_LEN)
                pointer++;
            else
                return END_OF_TAPE;
            break;
        case '<': //Move to the last index on the tape.
            if (pointer > 0)
                pointer--;
            else
                return END_OF_TAPE;
            break;
        case '+': //Increase the value of the current index on the tape by 1.
            if (tape[pointer] < MAX_CHAR_VALUE)
                tape[pointer]++;
            else
                return OVERFLOW;
            break;
        case '-': //Decrease the value of the current index on the tape by 1.
            if (tape[pointer] > 0)
                tape[pointer]--;
            else
                return UNDERFLOW;
            break;
        case '.': //Put the value of the current index on the tape in the output buffer.
            putchar(tape[pointer]);
            outBuffer[bufferCounter] = tape[pointer];
            bufferCounter++;
            break;
        case ',': //Get char from user and assign it to the current index on the tape.
            tape[pointer] = getchar();
            break;
        case '[': //Start of a while loop.
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
        case ']': //End of a while loop.
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
        prgPointer++; //Go to the next command of the BF program.
    }
    return 0;
}