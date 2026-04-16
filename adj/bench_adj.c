#define _POSIX_C_SOURCE 200809L

#include "adj_graph.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef SCAN_REPEATS
#define SCAN_REPEATS 5
#endif

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1e9;
}

/* Full traversal of all outgoing edges */
static uint64_t scan_graph(const AdjGraph * g)
{
    uint64_t acc = 0;

    for (uint32_t u = 0; u < g->n; ++u) {
        const AdjList *list = &g->adj[u];
        for (uint32_t i = 0; i < list->size; ++i) {
            acc += (uint64_t) u;
            acc += (uint64_t) list->edges[i].to;
            acc += (uint64_t) list->edges[i].w;
        }
    }

    return acc;
}

/* Touch only degree information */
static uint64_t degree_scan(const AdjGraph * g)
{
    uint64_t acc = 0;

    for (uint32_t u = 0; u < g->n; ++u) {
        acc += (uint64_t) g->adj[u].size;
    }

    return acc;
}

static void print_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <graph.gr>\n", prog);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    double t0 = now_sec();

    AdjGraph g;
    int rc = adj_graph_load_dimacs(filename, &g);
    if (rc != 0) {
        fprintf(stderr, "Failed to load graph, rc=%d\n", rc);
        return 1;
    }

    double t1 = now_sec();

    uint64_t deg_checksum = degree_scan(&g);

    double t2 = now_sec();

    uint64_t edge_checksum = 0;
    for (int r = 0; r < SCAN_REPEATS; ++r) {
        edge_checksum ^= scan_graph(&g);
    }

    double t3 = now_sec();

    printf("=== ADJ benchmark ===\n");
    printf("nodes             : %u\n", g.n);
    printf("edges             : %u\n", g.m);
    printf("load time         : %.6f s\n", t1 - t0);
    printf("degree-scan time  : %.6f s\n", t2 - t1);
    printf("edge-scan time    : %.6f s  (%d repeats)\n", t3 - t2, SCAN_REPEATS);
    printf("total time        : %.6f s\n", t3 - t0);
    printf("degree checksum   : %" PRIu64 "\n", deg_checksum);
    printf("edge checksum     : %" PRIu64 "\n", edge_checksum);

    adj_graph_destroy(&g);
    return 0;
}
