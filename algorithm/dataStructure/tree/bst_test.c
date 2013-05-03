#include <stdio.h>

#include "bst.h"

int int_comp(void *d1, void *d2)
{
    int a = *(int *)d1;
    int b = *(int *)d2;

    return a - b;
}

int visit(void *d)
{
    printf("%d ", *(int *)d);

    return 0;
}

//int data[] = {15, 6, 3, 7, 2, 4, 13, 9, 18, 17, 20};
int data[] = {15, 6, 3, 7, 2, 4, 13, 9, 18, 17, 20, 1, 8, 10, 25, 100, 5, 12, 11};

int main()
{
    struct bst t;
    int i = 0;

    bst_init(&t, int_comp);
    for(i = 0; i < sizeof(data)/sizeof(data[0]); ++i) {
        bst_insert(&t, &data[i]);
    }

    bst_walk_inorder(&t, visit);
    printf("\n");

    printf("minimum %d\n", *(int *)bst_minimum(&t));
    printf("maximum %d\n", *(int *)bst_maximum(&t));

    printf("succesor of 15 is %d\n", *(int *)bst_successor(&t, &data[0]));
    printf("predecessor of 15 is %d\n", *(int *)bst_predecessor(&t, &data[0]));

    for(i = 0; i < sizeof(data)/sizeof(data[0])/2; ++i) {
        bst_delete(&t, &data[i]);
    }
    bst_walk_inorder(&t, visit);
    printf("\n");


    return 0;
}
