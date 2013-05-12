#include <stdio.h>
#include <stdlib.h>

#include "bst.h"

struct bst_node {
    struct bst_node *left;
    struct bst_node *right;
    struct bst_node *parent;
    void *data;
};

#define L(n) (n)->left
#define R(n) (n)->right
#define P(n) (n)->parent

int bst_init(struct bst *t, compare_func comp)
{
    t->root = NULL;
    t->comp = comp;

    return comp ? 0 : -1;
}

static int _bst_walk_inorder(struct bst_node *n, visit_func v)
{
    if(!n) return 0;

    _bst_walk_inorder(L(n), v);
    v(n->data);
    _bst_walk_inorder(R(n), v);

    return 0;
}

int bst_walk_inorder(struct bst *t, visit_func v)
{
    if(t && t->root) return _bst_walk_inorder(t->root, v);

    return 0;
}

static struct bst_node *_bst_search_recursive(struct bst_node *n,
                                                compare_func comp,
                                                void *d)
{
    if(n){
        int c = comp(d, n->data);
        if(c == 0) {
            return n;
        }
        else if(c > 0) {
            return _bst_search_recursive(R(n), comp, d);
        }
        else {
            return _bst_search_recursive(L(n), comp, d);
        }
    }

    return NULL;
}

static struct bst_node *_bst_search_iterative(struct bst_node *n,
                                              compare_func comp,
                                              void *d)
{
    int c = -1;

    while(n && (c = comp(d, n->data)) != 0) {
        if(c > 0) n = R(n);
        else n = L(n);
    }

    if(n && 0 == c) return n;

    return NULL;
}

void *bst_search(struct bst *t, void *d)
{
    struct bst_node *n = NULL;
    if(t && t->root) {
        n = _bst_search_recursive(t->root, t->comp, d);
    }
    return n ? n->data : NULL;
}

static struct bst_node *_bst_minimum(struct bst_node *n)
{
    while(L(n)) n = L(n);

    return n;
}

void *bst_minimum(struct bst *t)
{
    struct bst_node *n = NULL;
    if(t && t->root) {
        n = _bst_minimum(t->root);
    }
    return n ? n->data : NULL;
}

static struct bst_node *_bst_maximum(struct bst_node *n)
{
    while(R(n)) n = R(n);

    return n;
}

void *bst_maximum(struct bst *t)
{
    struct bst_node *n = NULL;
    if(t && t->root) {
        n = _bst_maximum(t->root);
    }
    return n ? n->data : NULL;
}

static struct bst_node *_bst_successor(struct bst_node *n)
{
    if(R(n)) return _bst_minimum(R(n));

    struct bst_node *p = P(n);

    while(p && L(p) != n) {
        n = p;
        p = P(p);
    }

    return p;
}

void *bst_successor(struct bst *t, void *d)
{
    struct bst_node *n = NULL;

    if(t && t->root){
        n = _bst_successor(_bst_search_recursive(t->root, t->comp, d));
    }

    return n ? n->data : NULL;
}

static struct bst_node *_bst_predecessor(struct bst_node *n)
{
    if(L(n)) return _bst_maximum(L(n));

    struct bst_node *p = P(n);

    while(p && R(p) != n) {
        n = p;
        p = P(p);
    }

    return p;
}

void *bst_predecessor(struct bst *t, void *d)
{
    struct bst_node *n = NULL;

    if(t && t->root) {
        n = _bst_predecessor(_bst_search_recursive(t->root, t->comp, d));
    }

    return n ? n->data : NULL;
}

int bst_insert(struct bst *t, void *d)
{
    struct bst_node *n = t->root;
    struct bst_node *p = NULL;

    while(n) {
        p = n;
        /* in this case, c must not be equal to 0 */
        n = t->comp(d, n->data) < 0 ? L(n) : R(n);
    }

    struct bst_node *nnode = calloc(1, sizeof(struct bst_node));
    nnode->data = d;
    P(nnode) = p;

    if(NULL == p) t->root = nnode;
    else if(t->comp(nnode->data, p->data) > 0) {
        R(p) = nnode;
    }
    else {
        L(p) = nnode;
    }

    return 0;
}

void *bst_delete(struct bst *t, void *d)
{
    struct bst_node *n = _bst_search_recursive(t->root, t->comp, d);
    struct bst_node *dn = NULL;
    struct bst_node *child = NULL;
    void *data = NULL;
    if(!n) return NULL;

    data = n->data;

    if(NULL == L(n) || NULL == R(n)) dn = n;
    else dn = _bst_successor(n);

    if(L(dn)) child = L(dn);
    else child = R(dn);

    if(child) P(child) = P(dn);

    if(P(dn) == NULL) t->root = child;
    else if(dn == L(P(dn))){
        L(P(dn)) = child;
    }
    else {
        R(P(dn)) = child;
    }

    if(dn != n) {
        n->data = dn->data;
    }

    free(dn);

    return data;
}
