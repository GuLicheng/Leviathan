/*
    C:\Program Files (x86)\Windows Kits\10\Source\10.0.18362.0\ucrt\stdlib
*/
#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include <stdint.h>     // for size_t     
#include <assert.h>     // for assert
#include <stdlib.h>     // for malloc
#include <string.h>     // for strcmp


#include "my_ultility.h"

/*-----------------------------qsort--------------------------------*/

void QsortImpl(void* base, int left, int right, int size, int (*cmp)(const void* a, const void* b))
{
    assert(base != NULL && size >= 1 && cmp != NULL);
    if (left >= right) return;
    char* pleft = (char*)base + left * size;
    char* pkey = (char*)base + (left + (right - left) / 2) * size;
    Swap(pleft, pkey, size);
    int last = left;
    char* plast = (char*)base + last * size;
    for (int i = left + 1; i <= right; ++i)
    {
        char* pi = (char*)base + i * size;
        if (cmp(pi, pleft) < 0)
        {
            ++last;
            plast = (char*)base + last * size;
            Swap(pi, plast, size);
        }
    }
    Swap(pleft, plast, size);
    QsortImpl(base, left, last - 1, size, cmp);
    QsortImpl(base, last + 1, right, size, cmp);
}

/*
    be consistend with qsort
*/
void Qsort(void *bast, size_t num_of_element, size_t size,int (*cmp)(const void *,const void *))
{
    QsortImpl(bast, 0, num_of_element - 1, size, cmp);
/*
    int arr[] = {1, 4, 3, 5, 7, 2, 6, 9, 4, 2};
    int len = sizeof arr / sizeof arr[0];
    Qsort(arr, len, sizeof 0, CmpInt);
    
    const char* arr[] = {
        "132", "456", "1", "5555", "38"
    };
    Qsort(arr, 5, sizeof (char*), CmpString);
    for (int i = 0; i < 5; ++i)
    {
        puts(arr[i]);
    }

*/
}



/*---------------------------binary search--------------------------*/
void* LowerBound(void* base, int len, int size, const void* key, int (*cmp)(const void* a, const void* b))
{
    assert(base != NULL && len >= 0 && size >= 1 && cmp != NULL);
    int low = 0;
    int high = len;
    char* pmid = NULL;
    while (low < high)
    {
        int mid = low + ((high - low) >> 1);
        pmid = (char*)base + mid * size;
        if (cmp(pmid, key) < 0) { 
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }
    return (char*)base + low * size;
}

void* UpperBound(void* base, int len, int size, const void* key, int (*cmp)(const void* a, const void* b))
{
    assert(base != NULL && len >= 0 && size >= 1 && cmp != NULL);
    int low = 0;
    int high = len;
    char* pmid = NULL;
    while (low < high)
    {
        int mid = low + ((high - low) >> 1);
        pmid = (char*)base + mid * size;
        if (cmp(pmid, key) <= 0) { 
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }
    return (char*)base + low * size;
}

void* Bsearch(void* base, int len, int size, const void* key, int (*cmp)(const void* a, const void* b))
{
    void* res = LowerBound(base, len, size, key, cmp);
    if ((char*)res - (char*)base == len * size)
    {
        /* end */
        return NULL;
    }
    int c = cmp(res, key);
    if (c == 0) 
    {
        return res;
    }
    return NULL;
}


/*-------------------------------rand-------------------------------*/
struct RandomSeed
{
    unsigned int seed;
};

static struct RandomSeed* rd = NULL;

void Srand(uint32_t seed)
{
    if (!rd)
    {
        rd = (struct RandomSeed*)malloc(sizeof (struct RandomSeed*));
    }
    rd->seed = seed;
}

int Rand()
{
    if (!rd)
    {
        Srand(0);
    }
    rd->seed = rd->seed * 214013 + 2531011;
    return (rd->seed >> 16) & 0x7fff;
}

/*to be debugged*/
void* Lfind(void* base, int len, int size, const void* key, int (*cmp)(const void* a, const void* b))
{
    assert(base != NULL && len > 0 && size > 0 && key != NULL && cmp != NULL);
    char* first = (char*)base;
    char* last = first + len * size;
    for (char * p = first; p != last; p += size)
    {
        if (cmp(key, (char*)p) == 0)
        {
            return (char*)p;
        }
    }
    return NULL;
}

#endif