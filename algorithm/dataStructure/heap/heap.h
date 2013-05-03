#ifndef _HEAP_H_
#define _HEAP_H_


struct heap {
    int *data;
    int size;
    int cap;
};

int min_heap_init(struct heap *h, int cap);
int min_heap_fini(struct heap *h);
int min_heap_insert(struct heap *h, int value);
int min_heap_root(struct heap *h, int *value);
int min_heap_replace_root(struct heap *h, int value);
int min_heap_iterate(struct heap *h, int (*iter)(int value));

#endif
