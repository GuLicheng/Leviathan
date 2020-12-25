/*
    There just some macors so your must import the stdio.h before this header
*/
#ifndef __PRINT_H__
#define __PRINT_H__

#define printi(x) printf("%d", x)
#define puti(x) printf("%d\n", x)
#define printiarr(arr, size) do {     \
    putchar('[');                           \
    for (int i = 0; i < size; ++i)          \
        if (i == 0) printf("%d", arr[i]);   \
        else printf(", %d", arr[i]);        \
    putchar(']');                           \
} while (0)



#define printf(x) printf("%f", x)
#define printd(x) printf("%g", x)
#define printld(x) printf("%Lf", x)

#define put_float(x) printf("%f\n", x)
#define put_double(x) printf("%g\n", x)
#define put_long_double(x) printf("%Lf\n", x)

#define print_string(x) printf("%s", x)


#endif