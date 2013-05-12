#include <stdio.h>
#include <stdlib.h>

#define L(n) (n)->left
#define R(n) (n)->right
#define K(n) (n)->key
#define H(n) (n)->height
#define HH(n) ((n) ? (n)->height : 0)
#define BF(n) ((n) ? (HH((n)->left) - HH((n)->left)) : 0) //balance factor

#define MAX(a, b) (a) > (b) ? (a) : (b)

struct avl_node {
    struct avl_node *left;
    struct avl_node *right;
    int key;
    int depth;
};

int avl_init(struct avl *t)
{
    t->root = NULL;

    return 0;
}

static int _avl_walk_inorder(struct avl_node *n)
{
    if(!n) return 0;
    _avl_walk_inorder(L(n));
    printf("%d ", n->key);
    _avl_walk_inorder(R(n));
    return 0;
}

int avl_walk_inorder(struct avl *t)
{
    if(t && t->root) _avl_walk_inorder(t->root);
    printf("\n");
    return 0;
}

static int _avl_walk_preorder(struct avl_node *n)
{
    if(!n) return 0;
    printf("%d ", n->key);
    _avl_walk_inorder(L(n));
    _avl_walk_inorder(R(n));
    return 0;
}

int avl_walk_preorder(struct avl *t)
{
    if(t && t->root) _avl_walk_preorder(t->root);
    printf("\n");
    return 0;
}

static struct avl_node *_avlNewNode(int key)
{
    struct avl_node *n = malloc(sizeof(struct node));
    if(!n) return NULL;

    L(n) = R(n) = NULL;
    H(n) = 1;
    K(n) = key;

    return 0;
}

static struct avl_node *_rightRotate(struct avl_node *n)
{
    struct avl_node *t = L(n);

    L(n) = R(t);
    R(t) = n;

    H(t) = MAX(HH(t->left), HH(t->right)) + 1;
    H(n) = MAX(HH(n->left), HH(n->right)) + 1;

    return t;
}

static struct avl_node *_leftRotate(struct avl_node *n)
{
    struct avl_node *t = R(n);

    R(n) = L(t);
    L(t) = n;

    H(t) = MAX(HH(t->left), HH(t->right)) + 1;
    H(n) = MAX(HH(n->left), HH(n->right)) + 1;

    return t;
}

static struct avl_node *_avlInsert(struct avl_node *n, int key)
{
    if(!n) return _avlNewNode(key);
    
    if(key < KEY(n)) {
        L(n) = _avlInsert(L(n), key);
    }
    else {
        R(n) = _avlInsert(R(n), key);
    }
    H(n) = MAX(HH(n->left), HH(n->right)) + 1;

    int bf = BF(n);

    if(bf > 1) {
        if(key > K(L(n))) { // LR case
            L(n) = _leftRotate(L(n));
        }
        // else LL case
        return _rightRotate(n); 
    }
    // rr case
    if(bf < -1) {
        if(key < K(R(n))) {
            R(n) = _rightRotate(n); 
        }
        return _leftRotate(n); 
    }

    return n;
}

int avlInsert(struct avl *t, int key)
{
    if(!t) return -1;

    t->root = _avlInsert(t->root, key);

    return 0;
}
