#include <stdio.h>


void heapsort_knuth(int *a, int n);


static void print_array(const int *a, int n)
{
    for (int k = 0; k < n; k++)
        printf("%d ", a[k]);
    putchar('\n');
}


int main(void)
{
    int a[] = { 42, 17, 8, 99, 23, 5, 77, 1, 12 };
    int n = (int) (sizeof(a) / sizeof(a[0]));

    print_array(a, n);
    heapsort_knuth(a, n);
    print_array(a, n);

    return 0;
}
