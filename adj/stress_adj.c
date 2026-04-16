#define _POSIX_C_SOURCE 200809L

#include "adj_graph.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* Tocca davvero la memoria del grafo */
static uint64_t touch_adj_graph(const AdjGraph *g) {
    uint64_t acc = 0;

    for (uint32_t u = 0; u < g->n; ++u) {
        const AdjList *list = &g->adj[u];
        acc += list->size;
        acc += list->capacity;
        for (uint32_t i = 0; i < list->size; ++i) {
            acc += list->edges[i].to;
            acc += list->edges[i].w;
        }
    }

    return acc;
}

typedef struct {
    AdjGraph *graphs;
    size_t size;
    size_t capacity;
} GraphVec;

static int graphvec_push(GraphVec *v, AdjGraph g) {
    if (v->size == v->capacity) {
        size_t new_cap = (v->capacity == 0) ? 4 : v->capacity * 2;
        AdjGraph *tmp = realloc(v->graphs, new_cap * sizeof(AdjGraph));
        if (!tmp) {
            return 1;
        }
        v->graphs = tmp;
        v->capacity = new_cap;
    }

    v->graphs[v->size++] = g;
    return 0;
}

static void graphvec_destroy(GraphVec *v) {
    if (!v) return;
    for (size_t i = 0; i < v->size; ++i) {
        adj_graph_destroy(&v->graphs[i]);
    }
    free(v->graphs);
    v->graphs = NULL;
    v->size = 0;
    v->capacity = 0;
}

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <graph.gr> [max_copies]\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    size_t max_copies = 0; /* 0 = nessun limite */

    if (argc == 3) {
        char *end = NULL;
        unsigned long long x = strtoull(argv[2], &end, 10);
        if (!end || *end != '\0') {
            fprintf(stderr, "Invalid max_copies: %s\n", argv[2]);
            return 1;
        }
        max_copies = (size_t)x;
    }

    GraphVec vec = {0};
    uint64_t global_checksum = 0;
    double t0 = now_sec();

    for (size_t iter = 0; max_copies == 0 || iter < max_copies; ++iter) {
        AdjGraph g;
        int rc = adj_graph_load_dimacs(filename, &g);
        if (rc != 0) {
            fprintf(stderr, "\nLoad failed at copy %zu (rc=%d)\n", iter + 1, rc);
            break;
        }

        uint64_t chk = touch_adj_graph(&g);
        global_checksum ^= chk;

        if (graphvec_push(&vec, g) != 0) {
            fprintf(stderr, "\nFailed to store graph copy %zu\n", iter + 1);
            adj_graph_destroy(&g);
            break;
        }

        double now = now_sec();
        printf("ADJ copy %zu loaded: n=%u m=%u checksum=%" PRIu64 " elapsed=%.3f s\n",
               vec.size, g.n, g.m, chk, now - t0);
        fflush(stdout);
    }

    printf("\nADJ total copies kept in memory: %zu\n", vec.size);
    printf("ADJ global checksum: %" PRIu64 "\n", global_checksum);

    graphvec_destroy(&vec);
    return 0;
}
