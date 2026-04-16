#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Questa struttura rappresenta le informazioni minime associate a un nodo.
 *
 * In questo esempio non vogliamo costruire un grafo completo e realistico:
 * ci basta avere abbastanza dati da mostrare come organizzare la memoria.
 *
 * Ogni nodo ha:
 * - `id`: l'identificatore del nodo;
 * - `degree`: quanti archi uscenti possiede.
 */
typedef struct {
    uint32_t id;
    uint32_t degree;
} NodeHeader;


/*
 * Questa e` la struttura piu` importante dell'esempio.
 *
 * Idea chiave:
 * non facciamo tre `malloc` separate per `nodes`, `offsets` ed `edges`.
 * Facciamo una sola `malloc`, la salviamo in `block`, e poi "ritagliamo"
 * quel blocco in tre zone logiche.
 *
 * Quindi:
 * - `block` e` la vera memoria posseduta;
 * - `nodes`, `offsets` ed `edges` non possiedono memoria propria;
 * - sono solo puntatori che indicano dove iniziano le varie sezioni
 *   all'interno di `block`.
 *
 * Pensala cosi`:
 *
 *     block
 *     +---------------------------------------------------------------+
 *     | nodes | eventuale padding | offsets | eventuale padding |edges|
 *     +---------------------------------------------------------------+
 *     ^                           ^                             ^
 *     |                           |                             |
 *   nodes                      offsets                        edges
 */
typedef struct {
    size_t total_size;
    uint32_t node_count;
    uint32_t edge_count;
    NodeHeader *nodes;
    uint32_t *offsets;
    uint32_t *edges;
    unsigned char *block;
} GraphBlock;


/*
 * Questa funzione serve a rispettare l'allineamento dei tipi.
 *
 * Problema:
 * se metto un array di `uint32_t` o di `struct` in un indirizzo sbagliato,
 * su alcune architetture posso avere accessi inefficienti o addirittura
 * comportamento non valido.
 *
 * Soluzione:
 * dato un offset generico, lo spostiamo in avanti fino al prossimo multiplo
 * di `alignment`.
 *
 * Esempio mentale:
 * - se `value = 10`
 * - e `alignment = 4`
 * il prossimo indirizzo valido e` 12.
 */
static size_t align_up(size_t value, size_t alignment)
{
    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}


/*
 * Qui costruiamo manualmente tutto il layout in memoria.
 *
 * Questo e` il cuore della tecnica "una sola grossa allocazione".
 *
 * Vogliamo ottenere un blocco fatto cosi`:
 *
 *   [ NodeHeader[node_count] ]
 *   [ uint32_t offsets[node_count + 1] ]
 *   [ uint32_t edges[edge_count] ]
 *
 * Ma invece di dichiarare tre blocchi indipendenti, facciamo cosi`:
 *
 * 1. partiamo da `offset = 0`;
 * 2. decidiamo dove mettere `nodes`;
 * 3. avanzamo l'offset;
 * 4. decidiamo dove mettere `offsets`;
 * 5. avanzamo di nuovo;
 * 6. decidiamo dove mettere `edges`;
 * 7. la posizione finale dell'offset ci dice quanti byte servono in totale.
 *
 * In pratica prima facciamo "il progetto" del layout, poi facciamo la `malloc`.
 */
static int graph_block_init(GraphBlock * graph, uint32_t node_count, uint32_t edge_count)
{
    size_t offset = 0;
    size_t nodes_offset;
    size_t offsets_offset;
    size_t edges_offset;
    size_t total_size;

    /*
     * Prima sezione: i metadati dei nodi.
     *
     * `nodes_offset` dice a che byte del blocco inizieranno i `NodeHeader`.
     * Poi aggiorniamo `offset` per dire: "da qui in avanti questa zona e`
     * occupata, la prossima sezione deve partire dopo".
     */
    nodes_offset = align_up(offset, _Alignof(NodeHeader));
    offset = nodes_offset + node_count * sizeof(NodeHeader);

    /*
     * Seconda sezione: l'array `offsets`.
     *
     * Questo array e` tipico delle rappresentazioni stile CSR:
     * `offsets[u]` dice da dove iniziano gli archi del nodo `u`,
     * `offsets[u + 1]` dice dove finiscono.
     *
     * Serve `node_count + 1` proprio per poter rappresentare anche la fine
     * dell'ultima lista di adiacenza.
     */
    offsets_offset = align_up(offset, _Alignof(uint32_t));
    offset = offsets_offset + (node_count + 1) * sizeof(uint32_t);

    /*
     * Terza sezione: tutti gli archi in un unico array contiguo.
     *
     * Questo e` il vantaggio della tecnica:
     * gli archi non sono sparsi in tanti pezzi di memoria separati,
     * ma stanno tutti vicini in un solo buffer.
     */
    edges_offset = align_up(offset, _Alignof(uint32_t));
    offset = edges_offset + edge_count * sizeof(uint32_t);

    /* Alla fine `offset` coincide con la dimensione totale del blocco. */
    total_size = offset;

    /*
     * Questa e` l'unica allocazione reale.
     *
     * Dopo questa riga, tutta la struttura del grafo vivra` dentro `block`.
     */
    graph->block = malloc(total_size);
    if (!graph->block) {
        return 1;
    }

    /*
     * Azzeriamo il blocco per avere memoria pulita.
     * Non e` obbligatorio per il concetto, ma rende il demo piu` prevedibile.
     */
    memset(graph->block, 0, total_size);

    /*
     * Qui avviene la "magia pratica":
     * convertiamo i numeri `nodes_offset`, `offsets_offset`, `edges_offset`
     * in veri puntatori C.
     *
     * Esempio:
     * - `graph->block` punta all'inizio del buffer;
     * - `graph->block + nodes_offset` punta all'inizio della sezione nodes;
     * - facendo il cast a `NodeHeader *`, diciamo al compilatore come leggere
     *   quei byte.
     *
     * Da questo momento in poi possiamo usare `graph->nodes[i]`,
     * `graph->offsets[i]` e `graph->edges[i]` come se fossero array normali,
     * anche se in realta` stanno tutti dentro un solo blocco.
     */
    graph->total_size = total_size;
    graph->node_count = node_count;
    graph->edge_count = edge_count;
    graph->nodes = (NodeHeader *) (graph->block + nodes_offset);
    graph->offsets = (uint32_t *) (graph->block + offsets_offset);
    graph->edges = (uint32_t *) (graph->block + edges_offset);
    return 0;
}


/*
 * Questa funzione mostra un vantaggio importante della tecnica.
 *
 * Siccome tutta la memoria e` in un solo blocco, basta una sola `free`.
 * Non servono tre `free` separate.
 *
 * Dopo la `free` azzeriamo anche i puntatori per lasciare la struttura
 * in uno stato coerente e non rischiare usi accidentali di memoria liberata.
 */
static void graph_block_destroy(GraphBlock * graph)
{
    free(graph->block);
    graph->block = NULL;
    graph->nodes = NULL;
    graph->offsets = NULL;
    graph->edges = NULL;
    graph->total_size = 0;
    graph->node_count = 0;
    graph->edge_count = 0;
}


/*
 * Qui riempiamo il blocco con un esempio concreto.
 *
 * Il grafo e`:
 *
 *   0 -> 1, 2
 *   1 -> 2
 *   2 -> 0, 1
 *
 * L'array `nodes` contiene i metadati dei nodi.
 * L'array `edges` contiene tutti i vicini di tutti i nodi, uno dietro l'altro.
 * L'array `offsets` dice come spezzare `edges` per capire quali archi
 * appartengono a ciascun nodo.
 *
 * In questo caso:
 * - nodo 0 usa `edges[0..1]`
 * - nodo 1 usa `edges[2..2]`
 * - nodo 2 usa `edges[3..4]`
 *
 * Per questo `offsets` vale:
 * - offsets[0] = 0
 * - offsets[1] = 2
 * - offsets[2] = 3
 * - offsets[3] = 5
 */
static void graph_block_fill_example(GraphBlock * graph)
{
    graph->nodes[0].id = 0;
    graph->nodes[0].degree = 2;
    graph->nodes[1].id = 1;
    graph->nodes[1].degree = 1;
    graph->nodes[2].id = 2;
    graph->nodes[2].degree = 2;

    graph->offsets[0] = 0;
    graph->offsets[1] = 2;
    graph->offsets[2] = 3;
    graph->offsets[3] = 5;

    graph->edges[0] = 1;
    graph->edges[1] = 2;
    graph->edges[2] = 2;
    graph->edges[3] = 0;
    graph->edges[4] = 1;
}


/*
 * Questa funzione serve a "vedere" il layout in memoria.
 *
 * Prima stampiamo:
 * - l'indirizzo base del blocco unico;
 * - l'indirizzo della sezione `nodes`;
 * - l'indirizzo della sezione `offsets`;
 * - l'indirizzo della sezione `edges`.
 *
 * Se guardi gli indirizzi stampati a runtime, noterai che sono tutti vicini:
 * questo conferma che i dati vivono nello stesso buffer.
 *
 * Poi percorriamo il grafo con la logica CSR:
 * - per il nodo `u`, gli archi partono da `offsets[u]`
 * - e finiscono prima di `offsets[u + 1]`
 *
 * Quindi il sottoarray:
 *   edges[offsets[u]], ..., edges[offsets[u + 1] - 1]
 * e` la lista di adiacenza del nodo `u`.
 */
static void graph_block_print(const GraphBlock * graph)
{
    uint32_t u;

    printf("single block base: %p\n", (void *) graph->block);
    printf("nodes             : %p\n", (void *) graph->nodes);
    printf("offsets           : %p\n", (void *) graph->offsets);
    printf("edges             : %p\n", (void *) graph->edges);
    printf("total bytes       : %zu\n", graph->total_size);

    for (u = 0; u < graph->node_count; ++u) {
        uint32_t begin = graph->offsets[u];
        uint32_t end = graph->offsets[u + 1];
        uint32_t i;

        printf("node %u ->", graph->nodes[u].id);
        for (i = begin; i < end; ++i) {
            printf(" %u", graph->edges[i]);
        }
        printf("\n");
    }
}


int main(void)
{
    /*
     * Questo e` il programma di prova:
     * 1. crea un grafo dentro un blocco unico;
     * 2. riempie il blocco con dati di esempio;
     * 3. stampa sia il contenuto logico sia il layout in memoria;
     * 4. libera tutto con una sola `free`.
     */
    GraphBlock graph;

    if (graph_block_init(&graph, 3, 5) != 0) {
        fprintf(stderr, "Allocation failed.\n");
        return 1;
    }

    /* Qui il blocco esiste gia`: ora lo trattiamo come una struttura dati normale. */
    graph_block_fill_example(&graph);
    graph_block_print(&graph);
    graph_block_destroy(&graph);
    return 0;
}
