#include <stdlib.h>
#include <stdio.h>

#include <lv_c/list.h>

#define DEFAULT_CAPACITY 8



static void shrink_list(list_t* ls)
{
    if (ls->size >= ls->capacity >> 2) return;
    ls->capacity >>= 1;
    void** new_data = (void**)malloc(ls->capacity * sizeof(void*));
    for (size_t i = 0; i < ls->size; ++i)
        new_data[i] = ls->data[i];
    ls->data = new_data;
}

list_t* create_list()
{
    list_t* ls = (list_t*)calloc(1, sizeof(list_t*));
    ls->data = calloc(DEFAULT_CAPACITY, sizeof(void*));
    ls->size = 0;
    ls->capacity = DEFAULT_CAPACITY;
    return ls;
}

void push_back_list(list_t* ls, void* val)
{
    if (ls->size == ls->capacity)
    {
        // expand
        ls->capacity <<= 1;
        void** new_data = (void**)calloc(ls->capacity, sizeof(void*));
        for (size_t i = 0; i < ls->size - 1; ++i)
        {
            new_data[i] = ls->data[i];
        }
        ls->data = new_data;
    }
    ls->data[ls->size++] = val;
}

void pop_back_list(list_t* ls)
{
    free(ls->data[--ls->size]);
    shrink_list(ls);
}

list_t* destory_list(list_t* ls)
{
    for (size_t i = 0; i < ls->size; ++i)
        free(ls->data[i]);
    free(ls);
}




