#ifndef ADJ_GRAPH_H
#define ADJ_GRAPH_H

#include <stdint.h>

typedef struct {
    uint32_t to;
    uint32_t w;
} AdjEdge;

typedef struct {
    AdjEdge *edges;
    uint32_t size;
    uint32_t capacity;
} AdjList;

typedef struct {
    uint32_t n;
    uint32_t m;
    AdjList *adj;   // array of n adjacency lists
} AdjGraph;

/*
 * Load a DIMACS shortest-path graph (.gr) into adjacency-list format.
 *
 * Supported lines:
 *   c ...
 *   p sp <n> <m>
 *   a <u> <v> <w>
 *
 * Nodes in file are 1-based; stored graph is 0-based.
 *
 * Returns 0 on success, nonzero on error.
 */
int adj_graph_load_dimacs(const char *filename, AdjGraph *g);

/* Free all memory owned by the graph. */
void adj_graph_destroy(AdjGraph *g);

/* Print basic stats. */
void adj_graph_print_stats(const AdjGraph *g);

/* Print outgoing edges of node u (0-based). */
void adj_graph_print_node(const AdjGraph *g, uint32_t u, uint32_t max_edges);

/* Out-degree of node u. */
uint32_t adj_graph_out_degree(const AdjGraph *g, uint32_t u);

#endif
