#ifndef _BST_H_
#define _BST_H_

/*
 *  < 0 : less than
 *  > 0 : greater than
 *  = 0 : equal to
 */
typedef int (*compare_func)(void*, void*);
typedef int (*visit_func)(void*);

struct bst_node;
struct bst {
    struct bst_node *root;
    compare_func comp;
};

int bst_init(struct bst *t, compare_func comp);
int bst_walk_inorder(struct bst *t, visit_func v);
void *bst_search(struct bst *t, void *d);
void *bst_minimum(struct bst *t);
void *bst_maximum(struct bst *t);
void *bst_successor(struct bst *t, void *d);
void *bst_predecessor(struct bst *t, void *d);
int bst_insert(struct bst *t, void *d);
void *bst_delete(struct bst *t, void *d);


#endif
