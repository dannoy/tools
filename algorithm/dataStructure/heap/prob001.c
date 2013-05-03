// http://www.careercup.com/question?id=17693675
// How do you find the greatest 1000 elements in a list of a million elements?
// No other information given. What would be the runtime?
// Hint: You can do better than O(n log n).
// I didn't realize but it could be possible with Tree or Heaps.
//

#include <stdio.h>
#include "heap.h"

#define NELEMENTS 1000

int next_element(int *value)
{
    return scanf("%d", value);
}

int print_value(int value)
{
    fprintf(stdout, "%d\n", value);
    return 0;
}

int main()
{
    int i = 0, value, root;
    struct heap h;

    min_heap_init(&h, NELEMENTS);

    for(i = 0; i < NELEMENTS; ++i) {
        if(EOF == next_element(&value)) {
            return -1;
        }

        min_heap_insert(&h, value);
    }

    while(EOF != next_element(&value)) {
        if(min_heap_root(&h, &root) < 0) {
            return -1;
        }
        else if(value > root) {
            min_heap_replace_root(&h, value);
        }
    }

    //min_heap_iterate(&h, print_value);
    min_heap_fini(&h);


    return 0;
}
