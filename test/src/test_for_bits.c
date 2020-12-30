#include <stdio.h>
#include <complex.h>
#include <math.h>

#include "include/include/my_c/my_algorithm.h"
#include "include/include/my_c/macro/my_print.h"
#include "include/include/my_c/mt_bit.h"

int main()
{
 

    int c = HasSingleBit(1);
    put_int(c);
    c = HasSingleBit(2);
    put_int(c);
    c = HasSingleBit(3);
    c = HasSingleBit(4);
    put_int(c);
    return 0;
}
