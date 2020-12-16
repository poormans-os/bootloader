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
    SystemTable->ConOut->OutputString(SystemTable->ConOut, out);
    return true;
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
        size_t maxrem = 2 ^ 32 - written;

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
            int d = (char)va_arg(parameters, int /* char promotes to int */);
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
            int d = (char)va_arg(parameters, unsigned int /* char promotes to uint */);
            char tmp[32] = {0};
            if (!maxrem)
            {
                // TODO: Set errno to EOVERFLOW.
                return -1;
            }
            itoa(d, tmp, 10, true);
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
