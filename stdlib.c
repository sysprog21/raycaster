#include <stdlib.h>

char *itoa(int value, char *str, int base)
{
    char *res = str;
    if (value < 0) {
        *str++ = '-';
        value = -value;
    }
    char *curr = str;
    while (value > 0) {
        int digit = value % base;
        if (base == 16 && digit > 9)
            *curr = 'a' + digit - 10;
        else
            *curr = '0' + digit;
        ++curr;
        value /= base;
    }

    ptrdiff_t length = curr - str;
    for (int i = 0; i < length >> 1; ++i) {
        char c = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = c;
    }

    *curr = '\0';
    return res;
}