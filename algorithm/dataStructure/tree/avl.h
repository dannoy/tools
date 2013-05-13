#ifndef _AVL_H_
#define _AVL_H_

struct avl_node;
struct avl {
    struct avl_node *root;
};

int avl_init(struct avl *t);
int avl_walk_inorder(struct avl *t);
int avl_walk_preorder(struct avl *t);
int avl_insert(struct avl *t, int key);

#endif
