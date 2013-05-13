#include <stdio.h>
#include "avl.h"

int main()
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
