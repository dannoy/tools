/*
 * http://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
 * */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
 * T[i] represents the proper suffix length before W[i]
 * */
int kmp_table(char *W, int T[], int SIZE)
{
    int proper_suffix_length = 0;
    int T_idx = 2;
    assert(SIZE >= 2);

    T[0] = -1;
    T[1] = 0;

    while(T_idx < SIZE) {
        if(W[T_idx - 1] == W[proper_suffix_length]) {
            ++proper_suffix_length;
            T[T_idx] = proper_suffix_length;
            ++T_idx;
        }
        else if(proper_suffix_length > 0) {
            proper_suffix_length = T[proper_suffix_length];
        }
        else{
            T[T_idx] = 0;
            ++T_idx;
        }
    }

    void dump(){
        int i = 0;
        printf("T:\n");
        for(i = 0; i < SIZE; ++i) {
            printf("%d ", T[i]);
        }
        printf("\n");
    }
    //dump();

    return 0;
}

int kmp_search(char *S, char *W)
{
    int S_length = strlen(S);
    int W_length = strlen(W);
    int T[W_length];
    int m = 0, i = 0;

    kmp_table(W, T, W_length);

    while(m + i < S_length) {
        //printf("m %d i %d\n", m, i);
        if(S[m + i] == W[i]) {
            if(++i == W_length) {
                return m;
            }
            continue;
        }

        /*
         * Notice:
         *      when i == 0, T[i] == -1, so m = m + i - T[1] ==> m = m + i + 1
         */
        m = m + i - T[i];
        if(T[i] > -1) {
            i = T[i];
        }
        else {
            i = 0;
        }
    }

    return m;
}

int kmp_table_myself(char *W, int T[], int SIZE)
{
    int proper_suffix_length = 0;
    int T_idx = 2;

    assert(SIZE >= 2);

    T[0] = -1;
    T[1] = 0;

    while(T_idx < SIZE) {
        if(W[T_idx - 1] == W[proper_suffix_length]) {
            ++proper_suffix_length;
            T[T_idx] = proper_suffix_length;
            ++T_idx;
        }
        else {
            proper_suffix_length = 0;
            T[T_idx] = 0;
            ++T_idx;
        }
    }

    {
        int i = 0;
        for(i = 0; i < SIZE; ++i) {
            printf("%d ", T[i]);
        }
        printf("\n");
    }

    return 0;
}

int main()
{
    int pos = -1;

    pos = kmp_search("abc abcdab abcdabcdabde", "abcdabd");
    printf("pos %d\n",pos);

    return 0;
}
