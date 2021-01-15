#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct _List
{
    void** data;
    size_t size;
    size_t capacity;
}list_t;


typedef struct _List_Op
{
    list_t* (*create_list)();
    void (*push_back_list)(list_t* ls, void* val);
    void (*pop_back_list)(list_t *ls);
    list_t* (*destory_list)(list_t* ls);
}list_op_t;

list_op_t* get_list_option();





#endif // __LIST_H__