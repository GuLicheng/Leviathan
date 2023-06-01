#ifndef __BIT_H__
#define __BIT_H__

#include <stdint.h>

/*---------------------------rotate------------------------------------*/
uint64_t Rotl64(uint64_t value, int shift)
{
    shift &= 0x3f; // 0x3f = 63, keep shift <= 63
    value = (value >> (0x40 - shift)) | (value << shift); // 0x40 = 64, swap low and high bit
    return value;
}

uint32_t Rotl32(uint32_t value, int shift)
{
    shift &= 0x1f; // 0x1f = 31, keep shift <= 31
    value = (value >> (0x20 - shift)) | (value << shift); // 0x20 = 32 swap low and high bit
    return value;
}

uint64_t Rotr64(uint64_t value, int shift)
{
    shift &= 0x3f;
    value = (value << (0x40 - shift)) | (value >> shift);
    return value;
}

uint32_t Rotr32(uint32_t value, int shift)
{
    shift &= 0x1f;
    value = (value << (0x20 - shift)) | (value >> shift);
    return value;
}

int HasSingleBit(uint64_t n)
{
    // faster than n & n - 1 == 0
    return n > 0 && ((n & -n) == n);
}

// O(n), n is the number of 1
int PopCount(uint64_t n)
{
    int res = 0;
    while (n)
    {
        n &= n - 1;
        res++;
    }
    return res;
}

#endif