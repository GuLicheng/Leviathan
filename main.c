#include <lv_c/list.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct point2D
{
    int x;
    int y;
}point2D;

typedef struct point3D
{
    int x;
    int y;
    int z;
}point3D;

int main()
{
    list_t* ls = create_list();

    for (int i = 0; i < 100; ++i)
    {
        point2D* p = (point2D*)malloc(sizeof(point2D));
        p->x = p->y = i;
        push_back_list(ls, p);
        point2D* q = (point2D*)ls->data[i];
        printf("%d %d %d %d.\n", q->x, q->y, ls->size, ls->capacity);
    }
    for (int i = 0; i < 80; ++i)
        pop_back_list(ls);
    printf("%d %d.\n", ls->size, ls->capacity);
    destory_list(ls);

}
// gcc -I./ -o main ./*