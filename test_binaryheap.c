#include <assert.h>
#include <stddef.h>
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


static void test_empty_heap(void)
{
    struct binaryheap heap;

    binaryheap_init(&heap, int_cmp);

    assert(binaryheap_empty(&heap));
    assert(binaryheap_size(&heap) == 0);
    assert(binaryheap_peek(&heap) == NULL);
    assert(binaryheap_pop(&heap) == NULL);

    binaryheap_destroy(&heap);
}


static void test_push_pop_order(void)
{
    struct binaryheap heap;
    int values[] = {7, 3, 9, 1, 4, 8, 2};
    int expected[] = {1, 2, 3, 4, 7, 8, 9};
    size_t i;

    binaryheap_init (&heap, int_cmp);

    for (i = 0; i < sizeof (values) / sizeof (values[0]); i++)
        assert(binaryheap_push(&heap, &values[i]) == 0);

    assert(binaryheap_size(&heap) == sizeof(values) / sizeof(values[0]));
    assert(*(int *)binaryheap_peek(&heap) == 1);

    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
        assert(*(int *)binaryheap_pop(&heap) == expected[i]);

    assert(binaryheap_empty (&heap));

    binaryheap_destroy(&heap);
}


static void test_duplicates(void)
{
    struct binaryheap heap;
    int values[] = {5, 1, 5, 1, 3};
    int expected[] = {1, 1, 3, 5, 5};
    size_t i;

    binaryheap_init(&heap, int_cmp);

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
        assert(binaryheap_push(&heap, &values[i]) == 0);
    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
        assert(*(int *)binaryheap_pop(&heap) == expected[i]);

    binaryheap_destroy(&heap);
}


static void test_build(void)
{
    struct binaryheap heap;
    int values[] = {10, 6, 12, 2, 4};
    void *items[sizeof (values) / sizeof (values[0])];
    int expected[] = {2, 4, 6, 10, 12};
    size_t i;

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++)
        items[i] = &values[i];

    binaryheap_build(&heap, items, sizeof(items) / sizeof(items[0]), int_cmp);

    assert(binaryheap_size(&heap) == sizeof(items) / sizeof(items[0]));
    assert(*(int *)binaryheap_peek(&heap) == 2);

    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
        assert(*(int *)binaryheap_pop(&heap) == expected[i]);
}


int main(void)
{
    test_empty_heap();
    test_push_pop_order();
    test_duplicates();
    test_build();

    printf("All tests passed\n");
    return 0;
}
