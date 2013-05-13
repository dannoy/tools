#include <stdio.h>
#include "avl.h"

int test1()
{
    struct avl t;
    avl_init(&t);
    avl_insert(&t, 10);
    avl_insert(&t, 20);
    avl_insert(&t, 30);
    avl_insert(&t, 40);
    avl_insert(&t, 50);
    avl_insert(&t, 25);

    avl_walk_preorder(&t);
    avl_walk_inorder(&t);

    return 0;
}

int test2()
{
    struct avl t;
    avl_init(&t);
    avl_insert(&t, 9);
    avl_insert(&t, 5);
    avl_insert(&t, 10);
    avl_insert(&t, 0);
    avl_insert(&t, 6);
    avl_insert(&t, 11);
    avl_insert(&t, -1);
    avl_insert(&t, 1);
    avl_insert(&t, 2);

    avl_walk_preorder(&t);
    avl_walk_inorder(&t);

    avl_delete(&t, 10);
    //avl_delete(&t, -1);
    avl_walk_preorder(&t);
    avl_walk_inorder(&t);

    return 0;
}

int main()
{
    test1();
    test2();

    return 0;
}
