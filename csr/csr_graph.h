#ifndef CSR_GRAPH_H
#define CSR_GRAPH_H

#include <stddef.h>
#include <stdint.h>


typedef struct {
    uint32_t to;
    uint32_t w;
} Edge;


typedef struct {
    uint32_t n;                 // number of nodes
    uint32_t m;                 // number of edges
    uint32_t *off;              // size n+1
    Edge *edges;                // size m
} CsrGraph;


/*
 * Load a DIMACS shortest-path graph (.gr) into CSR format.
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
int csr_graph_load_dimacs(const char *filename, CsrGraph * g);


/* Free all memory owned by the graph. */
void csr_graph_destroy(CsrGraph * g);


/* Print basic stats. */
void csr_graph_print_stats(const CsrGraph * g);


/* Print outgoing edges of node u (0-based). */
void csr_graph_print_node(const CsrGraph * g, uint32_t u, uint32_t max_edges);


/* Out-degree of node u. */
uint32_t csr_graph_out_degree(const CsrGraph * g, uint32_t u);
#endif
