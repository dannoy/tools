#include <stdio.h>
#include <assert.h>

//正整数的无序分拆
int get_number_partition_count(int n, int max)
{
    if(n == 1 || max == 1) {
        return 1;
    }
    else if(n < max) {
        return get_number_partition_count(n, n);
    }
    else if(n == max) {
        return 1 + get_number_partition_count(n, n - 1);
    }
    else if(n > max) {
        return get_number_partition_count(n - max, max) +
                    get_number_partition_count(n, max - 1);
    }

    assert(0);
    return 0;
}

int main()
{
    int ret = 0;
    int n = 0;
    for(n = 1; n < 10; ++n) {
        int partition = get_number_partition_count(n, n);
        printf("%d : %d\n", n, partition);
    }

    return ret;
}
