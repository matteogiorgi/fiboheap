#include "csr_graph.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Alloca count * size byte inizializzati a zero e restituisce il puntatore risultante. */
static void *xcalloc(size_t count, size_t size)
{
    void *p = calloc(count, size);
    return p;
}

/* Alloca size byte non inizializzati e restituisce il puntatore risultante. */
static void *xmalloc(size_t size)
{
    void *p = malloc(size);
    return p;
}

/* Porta il grafo nello stato vuoto azzerando dimensioni e puntatori interni. */
static void csr_graph_zero(CsrGraph * g)
{
    g->n = 0;
    g->m = 0;
    g->off = NULL;
    g->edges = NULL;
}

/* Libera la memoria posseduta dal grafo e lo reimposta a uno stato vuoto e consistente. */
void csr_graph_destroy(CsrGraph * g)
{
    if (!g)
        return;
    free(g->off);
    free(g->edges);
    csr_graph_zero(g);
}

/* Restituisce il grado uscente del nodo u; se il grafo o l'indice non sono validi restituisce 0. */
uint32_t csr_graph_out_degree(const CsrGraph * g, uint32_t u)
{
    if (!g || !g->off || u >= g->n)
        return 0;
    return g->off[u + 1] - g->off[u];
}

/* Stampa statistiche aggregate del grafo: numero di nodi, archi e min/media/max dei gradi uscenti. */
void csr_graph_print_stats(const CsrGraph * g)
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
        uint32_t d = csr_graph_out_degree(g, u);
        if (d < min_deg)
            min_deg = d;
        if (d > max_deg)
            max_deg = d;
        sum_deg += d;
    }

    double avg_deg = (double) sum_deg / (double) g->n;

    printf("  out-degree min/avg/max: %u / %.3f / %u\n", min_deg, avg_deg, max_deg);
}

/* Stampa gli archi uscenti del nodo u, fino a max_edges elementi, con un riepilogo se ce ne sono altri. */
void csr_graph_print_node(const CsrGraph * g, uint32_t u, uint32_t max_edges)
{
    if (!g || !g->off || !g->edges)
        return;
    if (u >= g->n) {
        printf("Node %u out of range [0, %u)\n", u, g->n);
        return;
    }

    uint32_t begin = g->off[u];
    uint32_t end = g->off[u + 1];
    uint32_t deg = end - begin;

    printf("Node %u: out-degree = %u\n", u, deg);

    uint32_t shown = 0;
    for (uint32_t i = begin; i < end; ++i) {
        printf("  %u -> %u  (w=%u)\n", u, g->edges[i].to, g->edges[i].w);
        ++shown;
        if (shown >= max_edges) {
            if (deg > max_edges) {
                printf("  ... (%u more edges)\n", deg - max_edges);
            }
            break;
        }
    }
}

/* Carica da file un grafo diretto in formato DIMACS e costruisce la rappresentazione CSR dentro g. */
int csr_graph_load_dimacs(const char *filename, CsrGraph * g)
{
    if (!filename || !g)
        return 1;
    csr_graph_zero(g);

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "fopen('%s') failed: %s\n", filename, strerror(errno));
        return 2;
    }

    /*
     * First pass:
     *   - read header p sp n m
     *   - count out-degrees
     */
    char line[256];
    uint32_t n = 0, m = 0;
    int saw_problem = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\0')
            continue;

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

    uint32_t *deg = xcalloc((size_t) n, sizeof(uint32_t));
    if (!deg) {
        fprintf(stderr, "Out of memory allocating degree array.\n");
        fclose(fp);
        return 6;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\0')
            continue;

        if (line[0] == 'a') {
            uint32_t u, v, w;
            if (sscanf(line, "a %u %u %u", &u, &v, &w) != 3) {
                fprintf(stderr, "Malformed arc line: %s", line);
                free(deg);
                fclose(fp);
                return 7;
            }

            if (u == 0 || u > n || v == 0 || v > n) {
                fprintf(stderr, "Arc endpoint out of range: %s", line);
                free(deg);
                fclose(fp);
                return 8;
            }

            deg[u - 1]++;
        }
    }

    uint32_t *off = xmalloc((size_t) (n + 1) * sizeof(uint32_t));
    if (!off) {
        fprintf(stderr, "Out of memory allocating offset array.\n");
        free(deg);
        fclose(fp);
        return 9;
    }

    off[0] = 0;
    for (uint32_t u = 0; u < n; ++u) {
        off[u + 1] = off[u] + deg[u];
    }

    if (off[n] != m) {
        fprintf(stderr, "Warning: header says m=%u, counted arcs=%u\n", m, off[n]);
        m = off[n];
    }

    Edge *edges = xmalloc((size_t) m * sizeof(Edge));
    if (!edges) {
        fprintf(stderr, "Out of memory allocating edges array.\n");
        free(off);
        free(deg);
        fclose(fp);
        return 10;
    }

    /*
     * We need per-node write cursors.
     * Start from off[u], then advance as we fill.
     */
    uint32_t *cur = xmalloc((size_t) n * sizeof(uint32_t));
    if (!cur) {
        fprintf(stderr, "Out of memory allocating cursor array.\n");
        free(edges);
        free(off);
        free(deg);
        fclose(fp);
        return 11;
    }

    for (uint32_t u = 0; u < n; ++u) {
        cur[u] = off[u];
    }

    /*
     * Second pass: fill edges.
     */
    rewind(fp);

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == 'c' || line[0] == 'p' || line[0] == '\n' || line[0] == '\0')
            continue;

        if (line[0] == 'a') {
            uint32_t u, v, w;
            if (sscanf(line, "a %u %u %u", &u, &v, &w) != 3) {
                fprintf(stderr, "Malformed arc line in second pass: %s", line);
                free(cur);
                free(edges);
                free(off);
                free(deg);
                fclose(fp);
                return 12;
            }

            uint32_t src = u - 1;
            uint32_t pos = cur[src]++;

            edges[pos].to = v - 1;
            edges[pos].w = w;
        }
    }

    fclose(fp);
    free(cur);
    free(deg);

    g->n = n;
    g->m = m;
    g->off = off;
    g->edges = edges;

    return 0;
}
