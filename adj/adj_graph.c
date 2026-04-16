#include "adj_graph.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void adj_graph_zero(AdjGraph * g)
{
    g->n = 0;
    g->m = 0;
    g->adj = NULL;
}

static int adj_list_reserve(AdjList * list, uint32_t new_cap)
{
    if (new_cap <= list->capacity) {
        return 0;
    }

    AdjEdge *tmp = realloc(list->edges, (size_t) new_cap * sizeof(AdjEdge));
    if (!tmp) {
        return 1;
    }

    list->edges = tmp;
    list->capacity = new_cap;
    return 0;
}

static int adj_list_push(AdjList * list, uint32_t to, uint32_t w)
{
    if (list->size == list->capacity) {
        uint32_t new_cap = (list->capacity == 0) ? 4 : (list->capacity * 2);
        if (new_cap < list->capacity) {
            return 1;           /* overflow */
        }
        if (adj_list_reserve(list, new_cap) != 0) {
            return 1;
        }
    }

    list->edges[list->size].to = to;
    list->edges[list->size].w = w;
    list->size++;
    return 0;
}

void adj_graph_destroy(AdjGraph * g)
{
    if (!g)
        return;

    if (g->adj) {
        for (uint32_t u = 0; u < g->n; ++u) {
            free(g->adj[u].edges);
        }
        free(g->adj);
    }

    adj_graph_zero(g);
}

uint32_t adj_graph_out_degree(const AdjGraph * g, uint32_t u)
{
    if (!g || !g->adj || u >= g->n)
        return 0;
    return g->adj[u].size;
}

void adj_graph_print_stats(const AdjGraph * g)
{
    if (!g)
        return;

    printf("Graph stats:\n");
    printf("  nodes: %u\n", g->n);
    printf("  edges: %u\n", g->m);

    if (g->n == 0)
        return;

    uint32_t min_deg = UINT32_MAX;
    uint32_t max_deg = 0;
    unsigned long long sum_deg = 0;

    for (uint32_t u = 0; u < g->n; ++u) {
        uint32_t d = g->adj[u].size;
        if (d < min_deg)
            min_deg = d;
        if (d > max_deg)
            max_deg = d;
        sum_deg += d;
    }

    double avg_deg = (double) sum_deg / (double) g->n;
    printf("  out-degree min/avg/max: %u / %.3f / %u\n", min_deg, avg_deg, max_deg);
}

void adj_graph_print_node(const AdjGraph * g, uint32_t u, uint32_t max_edges)
{
    if (!g || !g->adj)
        return;

    if (u >= g->n) {
        printf("Node %u out of range [0, %u)\n", u, g->n);
        return;
    }

    const AdjList *list = &g->adj[u];
    printf("Node %u: out-degree = %u\n", u, list->size);

    uint32_t shown = 0;
    for (uint32_t i = 0; i < list->size; ++i) {
        printf("  %u -> %u  (w=%u)\n", u, list->edges[i].to, list->edges[i].w);
        shown++;
        if (shown >= max_edges) {
            if (list->size > max_edges) {
                printf("  ... (%u more edges)\n", list->size - max_edges);
            }
            break;
        }
    }
}

int adj_graph_load_dimacs(const char *filename, AdjGraph * g)
{
    if (!filename || !g)
        return 1;
    adj_graph_zero(g);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "fopen('%s') failed: %s\n", filename, strerror(errno));
        return 2;
    }

    char line[256];
    uint32_t n = 0, m = 0;
    int saw_problem = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        if (line[0] == 'p') {
            char kind1[16], kind2[16];
            if (sscanf(line, "%15s %15s %u %u", kind1, kind2, &n, &m) != 4) {
                fprintf(stderr, "Malformed problem line: %s", line);
                fclose(fp);
                return 3;
            }

            if (strcmp(kind1, "p") != 0 || strcmp(kind2, "sp") != 0) {
                fprintf(stderr, "Unsupported DIMACS problem line: %s", line);
                fclose(fp);
                return 4;
            }

            saw_problem = 1;
            break;
        }
    }

    if (!saw_problem) {
        fprintf(stderr, "No DIMACS problem line found.\n");
        fclose(fp);
        return 5;
    }

    AdjList *adj = calloc((size_t) n, sizeof(AdjList));
    if (!adj) {
        fprintf(stderr, "Out of memory allocating adjacency lists.\n");
        fclose(fp);
        return 6;
    }

    uint32_t edges_read = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        if (line[0] == 'a') {
            uint32_t u, v, w;
            if (sscanf(line, "a %u %u %u", &u, &v, &w) != 3) {
                fprintf(stderr, "Malformed arc line: %s", line);
                for (uint32_t i = 0; i < n; ++i)
                    free(adj[i].edges);
                free(adj);
                fclose(fp);
                return 7;
            }

            if (u == 0 || u > n || v == 0 || v > n) {
                fprintf(stderr, "Arc endpoint out of range: %s", line);
                for (uint32_t i = 0; i < n; ++i)
                    free(adj[i].edges);
                free(adj);
                fclose(fp);
                return 8;
            }

            uint32_t src = u - 1;
            uint32_t dst = v - 1;

            if (adj_list_push(&adj[src], dst, w) != 0) {
                fprintf(stderr, "Out of memory while growing adjacency list.\n");
                for (uint32_t i = 0; i < n; ++i)
                    free(adj[i].edges);
                free(adj);
                fclose(fp);
                return 9;
            }

            edges_read++;
        }
    }

    fclose(fp);

    if (edges_read != m) {
        fprintf(stderr, "Warning: header says m=%u, actually read %u arcs\n", m, edges_read);
        m = edges_read;
    }

    g->n = n;
    g->m = m;
    g->adj = adj;

    return 0;
}
