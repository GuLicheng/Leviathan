#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct list
{
    void** data;
    size_t size;
    size_t capacity;
}list_t;


list_t* create_list();

void push_back_list(list_t* ls, void* val);

void pop_back_list(list_t *ls);

list_t* destory_list(list_t* ls);





#endif // __LIST_H__