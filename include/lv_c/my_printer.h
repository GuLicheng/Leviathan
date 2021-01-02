#ifndef __PRINTER_H__
#define __PRINTER_H__

typedef enum 
{
    DEFAULT,
    RED, 
    YELLOW, 
    BLUE, 
    BLACK, 
    GREEN, 
    MAGENTA,
    CYAN,
    WITHE,
    BOLD,
    HIGH_LIGHT,
    REDUCE_LIGHT,
    ITALIC,
    UNDER_SCORE,
    BAR,
    CLEAN_SCREEN,
}STYLE;

typedef struct
{
    int (*print_i)(int x);
    int (*print_f)(float x);
    int (*print_d)(double x);
    int (*print_ld)(long double x);
    int (*contorl)(STYLE c);
    int (*endl)(void);
    int (*print_s)(const char* str);
}printer;

printer* init_printer();


#endif