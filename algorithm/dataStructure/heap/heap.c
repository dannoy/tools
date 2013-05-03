#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "heap.h"

#define SWAP(a, b) \
    do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define PARENT(a) ((a) / 2)
#define LEFT_CHILD(a) (2*(a))
#define RIGHT_CHILD(a) (2*(a) + 1)

#define DEBUG
#ifdef DEBUG
#define D(fmt, ...) fprintf(stderr,fmt, __VA_ARGS__);
#else
#define D(fmt, ...)
#endif

static int int_smaller(int a, int b)
{
    return a < b ? 1 : 0;
}

static int int_bigger(int a, int b)
{
    return a > b ? 1 : 0;
}

static int heap_init(struct heap *h, int cap)
{
    assert(h != (void *)0);
    // We use index from 1 to cap to achive:
    // 1. left child of k sits in 2k
    // 2. right child of k sits in 2k+1
    // 3. parent of k sits in floor(k/2)
    h->data = malloc((cap + 1) * sizeof(int));
    h->size = 0;
    h->cap = cap;
    return 0;
}

static int heap_fini(struct heap *h)
{
    if(h && h->data) free(h->data);
    return 0;
}

static int heapify(struct heap *h, int node, int (*cond)(int, int))
{
    int cnode;
    int ret = 0;
    int l = LEFT_CHILD(node);
    int r = RIGHT_CHILD(node);

    if(l <= h->size
            &&cond(h->data[l], h->data[node])) {
        cnode = l;
    } // else must be cnode = node; so cond(l, node) is wrong
    else {cnode = node;}

    if(r <= h->size
            && cond(h->data[r], h->data[cnode])) {
        cnode = r;
    }

    if(cnode != node) {
        SWAP(h->data[cnode], h->data[node]);
        ret = heapify(h, cnode, cond);
    }

    return ret;
}

int min_heap_init(struct heap *h, int cap)
{
    return heap_init(h, cap);
}

int min_heap_fini(struct heap *h)
{
    return heap_fini(h);
}

int min_heap_insert(struct heap *h, int value)
{
    int i = -1;
    if(h->size >= h->cap) {
        return -1;
    }
    h->data[++h->size] = value;
    i = h->size;
    while(i > 1
            && int_smaller(h->data[i], h->data[PARENT(i)])) {
        SWAP(h->data[i], h->data[PARENT(i)]);
        i = PARENT(i);
    }
    //D("insert %d i %d\n", value, h->size);

    return 0;
}

int min_heap_replace_root(struct heap *h, int value)
{
    h->data[1] = value;
    return heapify(h, 1, int_smaller);
}

int min_heap_root(struct heap *h, int *value)
{
    if(h->size > 0) {
        *value = h->data[1];
        //D("root %d \n", *value);
        return 0;
    }
    return -1;
}

int min_heap_iterate(struct heap *h, int (*iter)(int value))
{
    int i = 1;
    for(i = 1; i <= h->size; ++i) {
        if(iter(h->data[i]) < 0) return -1;
    }

    return 0;
}

int max_heap_init(struct heap *h, int cap)
{
    return heap_init(h, cap);
}
