#include <stdio.h>
#include <stdlib.h>

#include "../my_printer.h"

static int print_i(int x)
{
    return printf("%d", x);
}

static int print_f(float x)
{
    return printf("%f", x);
}

static int print_d(double x)
{
    return printf("%f", x);
}

static int print_ld(long double x)
{
    return printf("%Lf", x);
}

static int change_style(STYLE c)
{
	switch (c)
	{
		case DEFAULT: printf("\033[0m"); break;
		case RED : printf("\033[31m"); break;
		case YELLOW : printf("\033[33m"); break; 
		case BLUE : printf("\033[34m"); break;
		case BLACK : printf("\033[30m"); break;
		case GREEN : printf("\033[32m"); break;
		case MAGENTA: printf("\033[35m"); break;
		case CYAN : printf("\033[36m"); break;
		case WITHE : printf("\033[37m"); break;
		case BOLD: printf("\033[1m"); break;
		case HIGH_LIGHT: printf("\033[1m"); break;
		case REDUCE_LIGHT: printf("\033[2m"); break;
		case ITALIC: printf("\033[3m"); break;
		case UNDER_SCORE: printf("\033[4m"); break;
		case BAR: printf("\033[9m"); break;
		case CLEAN_SCREEN: printf("\033[2J"); break;
		default: break;
	}
	
	return -1;
}

static int endl_with_flush()
{
	putchar('\n');
	return fflush(stdout);
}

static int print_s(const char* str)
{
	return printf("%s", str);
}

static printer p = 
{
    print_i,
    print_f,
    print_d,
    print_ld,
	change_style,
	endl_with_flush,
	print_s
};

printer* init_printer()
{
    return &p;
}




#if 0

#include <stdio.h>
#include <stdlib.h>
int main(int argc, char** argv) {
#ifdef _MSC_VER
#ifdef _WIN64
	fprintf(stdout, "size_t:%zu\n", sizeof(size_t));
	fprintf(stdout, "int:%zu\n", sizeof(int));
	fprintf(stdout, "long:%zu\n", sizeof(long));
	fprintf(stdout, "long int:%zu\n", sizeof(long int));
	fprintf(stdout, "long long int:%zu\n", sizeof(long long int));
#elif _WIN32
	fprintf(stdout, "size_t:%u\n", sizeof(size_t));
	fprintf(stdout, "int:%u\n", sizeof(int));
	fprintf(stdout, "long:%u\n", sizeof(long));
	fprintf(stdout, "long int:%u\n", sizeof(long int));
	fprintf(stdout, "long long int:%u\n", sizeof(long long int));
#endif
#else
#ifdef __x86_64__
	fprintf(stdout, "size_t:%lu\n", sizeof(size_t));
	fprintf(stdout, "int:%lu\n", sizeof(int));
	fprintf(stdout, "long:%lu\n", sizeof(long));
	fprintf(stdout, "long int:%lu\n", sizeof(long int));
	fprintf(stdout, "long long int:%lu\n", sizeof(long long int));
#elif __i386__
	fprintf(stdout, "size_t:%u\n", sizeof(size_t));
	fprintf(stdout, "int:%u\n", sizeof(int));
	fprintf(stdout, "long:%u\n", sizeof(long));
	fprintf(stdout, "long int:%u\n", sizeof(long int));
	fprintf(stdout, "long long int:%u\n", sizeof(long long int));
#endif
#endif;
	system("pause");
	return 0;
}

#endif