#include <stdio.h>
#include "binaryheap.h"


static int int_cmp(const void *lhs, const void *rhs)
{
    const int *left = lhs;
    const int *right = rhs;

    if (*left < *right)
        return -1;
    if (*left > *right)
        return 1;

    return 0;
}


int main(void)
{
    struct binaryheap heap;
    int values[] = {7, 3, 9, 1, 4, 8, 2};
    size_t i;

    binaryheap_init(&heap, int_cmp);

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        if (binaryheap_push(&heap, &values[i]) != 0) {
            fprintf(stderr, "binaryheap_push failed\n");
            binaryheap_destroy(&heap);
            return 1;
        }
    }

    while (!binaryheap_empty(&heap)) {
        int *value = binaryheap_pop(&heap);
        printf("%d\n", *value);
    }

    binaryheap_destroy(&heap);

    return 0;
}
