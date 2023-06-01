#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <assert.h>
#include <string.h>

/*------------------------------comparators---------------------------*/
int CmpString(const void* a, const void* b)
{
    assert(a != NULL && b != NULL);
    if (a != b) 
    {
        const char* lhs = *(const char**)a;
        const char* rhs = *(const char**)b;
        return strcmp(lhs, rhs); 
    }

    return 0;
}

int CmpInt(const void* a, const void* b)
{
    assert(a != NULL && b != NULL);
    const int* lhs = (const int*)a;
    const int* rhs = (const int*)b;
    // return *lhs - *rhs; this might UB for overflow
    if (*lhs < *rhs) return -1;
    
    if (*lhs == *rhs) return 0;
    
    return 1;
}

/*----------------------------------swap-----------------------*/
void Swap(const void* a, const void* b, int size)
{
    assert(a != NULL && b != NULL && size > 0);
    if (a != b)
    {
        int i = 0;
        while (size--)
        {
            //  swap one character at a time to avoid potential alignment problem
            const char tmp = *((char*)a + i);
            *((char*)a + i) = *((char*)b + i);
            *((char*)b + i) = tmp;
            ++i;
        }
    }
/*
    int a = 1;
    int b = 2;
    Swap(&a, &b, sizeof a);
    long double c = 2.864654876;
    long double d = 1.99999;
    Swap(&c, &d, sizeof c);
*/
}

#endif