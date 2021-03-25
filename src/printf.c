#include <Uefi.h>
#include <stdbool.h>
#include <stdarg.h>
#include "stdio.h"

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

void *memset(void *s, const int c, const size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((char *)s)[i] = (char)c;
    return s;
}

// Iterative function to implement itoa() function in C
char *itoa(int value, char *str, const int base, const bool uint)
{
    char *rc;
    char *ptr;
    char *low;
    // Check for supported base.
    if (base < 2 || base > 36)
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if (value < 0 && base == 10 && !uint)
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while (value);
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while (low < ptr)
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void toLString(CHAR16 *dst, const char *src, const size_t len)
{
    size_t i = 0;

    for (i = 0; i < len; i++)
    {
        dst[i] = (CHAR16)src[i];
    }
    dst[i + 1] = (CHAR16)'\0';
}

/*
The function prints a simple string and returns if the string was printed properly
const char* data - string to print
size_t length - length of string
*/
static bool print(const char *data, const size_t length)
{
    // if (EFI_SUCCESS == )
    //     return true;
    // return false;
    CHAR16 out[150] = {0};
    toLString(out, data, length);
    acquireMutex(&printfMutex);
    SystemTable->BootServices->Stall(1000);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
    releaseMutex(&printfMutex);
    return true;
}

int isspace(int c)
{
    return c == ' ';
}

int isalpha(int c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

int isdigit(int c)
{
    return ('0' <= c && c <= '9');
}

int isupper(int c)
{
    return ('A' <= c && c <= 'Z');
}

#define LONG_MAX ((long)(~0UL >> 1))
#define LONG_MIN (~LONG_MAX)
// https://code.woboq.org/gcc/libiberty/strtol.c.html
long strtol(const char *nptr, char **endptr, unsigned int base)
{
    const char *s = nptr;
    unsigned long acc = 0;
    unsigned int c = 0;
    unsigned long cutoff = 0;
    unsigned int neg = 0, any = 0, cutlim = 0;

    do
    {
        c = *s++;
    } while (isspace(c));
    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++)
    {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0)
        acc = neg ? LONG_MIN : LONG_MAX;
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

char *convert(unsigned long long num, const int base)
{
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num % base];
        num /= base;
    } while (num != 0);

    return (ptr);
}

/*
The function will print a string with option to other values(integer, float, char, string). 
returns how many bytes where written.
const char* restrict format - string to print
... - option to enter several parameters such as integer (%d), float (%f), char (%c), string (%s).
*/
int printf(const char *restrict format, ...)
{
    va_list parameters; //list of parameters
    va_start(parameters, format);

    int written = 0; //initialize 0 written bytes

    while (*format != '\0')
    {
        size_t maxrem = INT_MAX - written;

        if (format[0] != '%' || format[1] == '%') //check for entered parameters with %d
        {
            if (format[0] == '%')
                format++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%')
                amount++;
            if (maxrem < amount)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, amount))
                return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = format++;

        if (*format == 'c')
        {
            format++;
            char c = (char)va_arg(parameters, int /* char promotes to int */);
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(&c, sizeof(c)))
                return -1;
            written++;
        }
        else if (*format == 'x')
        {
            //FIXME - implement in a better way
            format++;
            unsigned long long d = (unsigned long long)va_arg(parameters, unsigned long long /* char promotes to int */);
            char *tmp = {0};
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            tmp = convert(d, 16);
            if (!print(tmp, strlen(tmp)))
                return -1;
            written++;
        }
        else if (*format == 'd')
        {
            //FIXME - implement in a better way
            format++;
            int d = (int)va_arg(parameters, int /* char promotes to int */);
            char tmp[32] = {0};
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            itoa(d, tmp, 10, false);
            if (!print(tmp, strlen(tmp)))
                return -1;
            written++;
        }
        else if (*format == 'u')
        {
            //FIXME - implement in a better way
            format++;
            unsigned int u = (unsigned int)va_arg(parameters, unsigned int /* char promotes to uint */);
            char tmp[32] = {0};
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            itoa(u, tmp, 10, true);
            if (!print(tmp, strlen(tmp)))
                return -1;
            written++;
        }
        else if (*format == 's')
        {
            format++;
            const char *str = va_arg(parameters, const char *);
            size_t len = strlen(str);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(str, len))
                return -1;
            written += len;
        }
        else
        {
            format = format_begun_at;
            size_t len = strlen(format);
            if (maxrem < len)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            if (!print(format, len))
                return -1;
            written += len;
            format += len;
        }
    }

    va_end(parameters);
    return written;
}

unsigned short kernelGetchar()
{

    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
    SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    putchar(Key.UnicodeChar);

    return Key.UnicodeChar;
}

unsigned char getchar()
{
    scanfPID = 1;
    while (scanfPID == 1)
    {
    }

    return scanfBuffer;
}

int scanf(const char *str, ...)
{
    // mutex
    acquireMutex(&scanfMutex);

    va_list vl;
    int i = 0, j = 0, ret = 0;
    char buff[100] = {0};
    char *out_loc;

    char temp = '\0';

    while (temp != 13)
    {
        temp = getchar();

        //Result in scanfBuffer
        if (temp)
        {
            buff[i] = temp;
            i++;
        }
    }
    va_start(vl, str);
    i = 0;
    while (str && str[i])
    {
        if (str[i] == '%')
        {
            i++;
            switch (str[i])
            {
            case 'c':
            {
                *(char *)va_arg(vl, char *) = buff[j];
                j++;
                ret++;
                break;
            }
            case 'd':
            {
                *(int *)va_arg(vl, int *) = (int)strtol(&buff[j], &out_loc, 10);
                j += out_loc - &buff[j];
                ret++;
                break;
            }
            case 'x':
            {
                *(int *)va_arg(vl, int *) = strtol(&buff[j], &out_loc, 16);
                j += out_loc - &buff[j];
                ret++;
                break;
            }
            }
        }
        else
        {
            buff[j] = str[i];
            j++;
        }
        i++;
    }
    va_end(vl);

    // mutex
    releaseMutex(&scanfMutex);
    return ret;
}

char *fgets(char *str, int n)
{
    for (size_t i = 0; i < n; i++)
    {
        str[i] = getchar();
        if (str[i] == 13)
            break;
    }

    return str;
}

int kernelScanf()
{
    while (1)
    {
        while (!scanfPID)
        {
        }

        scanfBuffer = kernelGetchar();

        scanfPID = 0;
    }
}
