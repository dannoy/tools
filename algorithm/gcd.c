#include <stdio.h>

int gcd1(int m, int n)
{
    int r = 0;
    r = m % n;
    while(r) {
        m = n;
        n = r;
        r = m % n;
    }

    return n;
}

int gcd2(int m, int n)
{
    int r = 1; /* Any value except 0 */
    while(r) {
        r = m % n;
        m = n;
        n = r;
    }

    return m;
}

int gcd3(int m, int n)
{
    while(1) {
        if(0 == (m = m % n)) {
            return n;
        }
        else if(0 == (n = n % m)) {
            return m;
        }
    }

    return 1;
}

int gcd4(int m, int n)
{
    if(0 == m % n) {
        return n;
    }

    return gcd4(n, m % n);
}

int main()
{
    int gcd = 0;

    gcd = gcd1(119, 544);
    printf("gcd1 is %d\n", gcd);
    gcd = gcd2(119, 544);
    printf("gcd2 is %d\n", gcd);
    gcd = gcd3(119, 544);
    printf("gcd3 is %d\n", gcd);
    gcd = gcd4(119, 544);
    printf("gcd4 is %d\n", gcd);

    return 0;
}
