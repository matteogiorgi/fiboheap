#include "csr_graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <graph.gr> [node_to_print]\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    uint32_t node_to_print = 0;

    if (argc >= 3) {
        char *end = NULL;
        unsigned long x = strtoul(argv[2], &end, 10);
        if (!end || *end != '\0') {
            fprintf(stderr, "Invalid node index: %s\n", argv[2]);
            return 1;
        }
        node_to_print = (uint32_t)x;
    }

    CsrGraph g;
    int rc = csr_graph_load_dimacs(filename, &g);
    if (rc != 0) {
        fprintf(stderr, "Failed to load graph, rc=%d\n", rc);
        return 1;
    }

    csr_graph_print_stats(&g);

    printf("\nSome example nodes:\n");
    csr_graph_print_node(&g, 0, 10);

    if (g.n > 1) {
        csr_graph_print_node(&g, 1, 10);
    }

    if (node_to_print < g.n) {
        csr_graph_print_node(&g, node_to_print, 20);
    } else {
        printf("Requested node %u is out of range, skipping.\n", node_to_print);
    }

    csr_graph_destroy(&g);
    return 0;
}
