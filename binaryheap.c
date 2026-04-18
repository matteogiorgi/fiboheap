#include <stdlib.h>
#include "binaryheap.h"


/* Scambia due puntatori memorizzati nell'array interno della heap. */
static void binaryheap_swap(void **lhs, void **rhs)
{
    void *tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}


/* Fa risalire un nodo finché la proprietà di min-heap non è ristabilita. */
static void binaryheap_sift_up(struct binaryheap *heap, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;

        if (heap->cmp(heap->data[index], heap->data[parent]) >= 0)
            break;

        binaryheap_swap(&heap->data[index], &heap->data[parent]);
        index = parent;
    }
}


/* Fa scendere un nodo scegliendo ogni volta il figlio più piccolo. */
static void binaryheap_sift_down(struct binaryheap *heap, size_t index)
{
    size_t left, right, smallest;

    for (;;) {
        left = 2 * index + 1, right = left + 1, smallest = index;

        if (left < heap->size && heap->cmp(heap->data[left], heap->data[smallest]) < 0)
            smallest = left;
        if (right < heap->size && heap->cmp(heap->data[right], heap->data[smallest]) < 0)
            smallest = right;
        if (smallest == index)
            break;

        binaryheap_swap(&heap->data[index], &heap->data[smallest]);
        index = smallest;
    }
}


/* Inizializza una heap vuota associandole il comparatore scelto. */
void binaryheap_init(struct binaryheap *heap, binaryheap_cmp_fn cmp)
{
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
    heap->cmp = cmp;
}


/* Libera il buffer interno della heap senza deallocare gli elementi puntati. */
void binaryheap_destroy(struct binaryheap *heap)
{
    free(heap->data);
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
    heap->cmp = NULL;
}


/* Garantisce una capacità minima per evitare riallocazioni ai push successivi. */
int binaryheap_reserve(struct binaryheap *heap, size_t capacity)
{
    void **new_data;

    if (capacity <= heap->capacity)
        return 0;
    if ((new_data = realloc(heap->data, capacity * sizeof(*heap->data))) == NULL)
        return -1;

    heap->data = new_data;
    heap->capacity = capacity;
    return 0;
}


/* Inserisce un elemento in fondo all'array e lo riporta nella posizione corretta. */
int binaryheap_push(struct binaryheap *heap, void *item)
{
    size_t new_capacity;

    if (heap->size == heap->capacity) {
        new_capacity = heap->capacity == 0 ? 8 : heap->capacity * 2;
        if (new_capacity < heap->capacity || binaryheap_reserve(heap, new_capacity) != 0)
            return -1;
    }

    heap->data[heap->size] = item;
    binaryheap_sift_up(heap, heap->size);
    heap->size++;
    return 0;
}


/* Restituisce l'elemento in cima alla heap senza rimuoverlo. */
void *binaryheap_peek(const struct binaryheap *heap)
{
    if (heap->size == 0)
        return NULL;

    return heap->data[0];
}


/* Estrae la radice della heap e ripristina l'ordine con sift_down. */
void *binaryheap_pop(struct binaryheap *heap)
{
    void *item;

    if (heap->size == 0)
        return NULL;

    item = heap->data[0];
    heap->size--;

    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        binaryheap_sift_down(heap, 0);
    }

    return item;
}


/* Restituisce il numero corrente di elementi memorizzati. */
size_t binaryheap_size(const struct binaryheap *heap)
{
    return heap->size;
}


/* Indica se la heap è vuota. */
int binaryheap_empty(const struct binaryheap *heap)
{
    return heap->size == 0;
}


/* Costruisce una heap valida a partire da un array già popolato. */
void binaryheap_build(struct binaryheap *heap, void **items, size_t count, binaryheap_cmp_fn cmp)
{
    size_t index;

    heap->data = items;
    heap->size = count;
    heap->capacity = count;
    heap->cmp = cmp;

    if (count < 2)
        return;

    for (index = count / 2; index > 0; index--)
        binaryheap_sift_down(heap, index - 1);
}
