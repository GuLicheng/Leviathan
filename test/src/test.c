#include <stdio.h>
#include "../include/my_c/my_printer.h"
#include "../include/my_c/mt_bit.h"

int main()
{
    printer *p = init_printer();
    p->print_i(~0);
    return 0;
}