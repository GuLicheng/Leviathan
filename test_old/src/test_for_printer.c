// #include "../include/my_c/src/my_printer.c"
#include "../../include/lv_c/my_printer.h"

int main()
{
    printer* p = init_printer();
    p->print_i(0);
    p->print_d(1.0);
    putchar('*');
}