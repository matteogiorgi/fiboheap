#include <stdio.h>


void heapsort_knuth(int *a, int n)
{
    int l, r, i, j, R;

    if (n <= 1)
        return;

    /* H1. Initialize */
    l = n / 2;
    r = n - 1;

  H2:
    if (l > 0) {
        l = l - 1;
        R = a[l];
    }
    else {
        R = a[r];
        a[r] = a[0];
        r = r - 1;
        if (r == 0) {
            a[0] = R;
            return;
        }
    }

    /* H3. Prepare for siftup */
    i = l;
    j = 2 * i + 1;

  H4:
    if (j > r)
        goto H8;

    if (j < r)
        goto H5;

    /* j == r */
    goto H6;

  H5:
    /* Find larger child */
    if (a[j] < a[j + 1])
        j = j + 1;

  H6:
    /* Larger than K? */
    if (R >= a[j])
        goto H8;

  H7:
    /* Move it up */
    a[i] = a[j];
    i = j;
    j = 2 * i + 1;
    goto H4;

  H8:
    /* Store R */
    a[i] = R;
    goto H2;
}


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
