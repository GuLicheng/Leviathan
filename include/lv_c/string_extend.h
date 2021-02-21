#ifndef __STRING_EXTEND_HPP__
#define __STRING_EXTEND_HPP__

#include <string.h>

void trim(char* string, int(*pred)(int))
{
    // empty string
    if (!string || !string[0])
        return;

    char* p;
    // trim left
    for (p = string; *p && pred(*p); ++p);
    char* left = p;

    // trim right
    for (p = string + strlen(string) - 1; p >= string && pred(*p); --p);

    // copy [left, right] to string
    memmove(string, left, p - left + 1);

    // set last charactor
    string[p - left + 1] = '\0';
}


/*
    char string1[] = "   3456 ";
    char string2[] = "";
    char string3[] = "3456";
    trim(string1, isspace);
    trim(string2, isspace);
    trim(string3, isspace);
    printf("%s\n", string1);
    printf("%s\n", string2);
    printf("%s\n", string3);
*/

#endif