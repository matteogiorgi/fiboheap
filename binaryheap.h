#ifndef BINARYHEAP_H
#define BINARYHEAP_H


#include <stddef.h>


typedef int (*binaryheap_cmp_fn)(const void *lhs, const void *rhs);
struct binaryheap {
    void **data;
    size_t size;
    size_t capacity;
    binaryheap_cmp_fn cmp;
};


void binaryheap_init(struct binaryheap *heap, binaryheap_cmp_fn cmp);
void binaryheap_destroy(struct binaryheap *heap);
int binaryheap_reserve(struct binaryheap *heap, size_t capacity);
int binaryheap_push(struct binaryheap *heap, void *item);
void *binaryheap_peek(const struct binaryheap *heap);
void *binaryheap_pop(struct binaryheap *heap);
size_t binaryheap_size(const struct binaryheap *heap);
int binaryheap_empty(const struct binaryheap *heap);
void binaryheap_build(struct binaryheap *heap, void **items, size_t count, binaryheap_cmp_fn cmp);
#endif
