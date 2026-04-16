#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Questa struttura implementa una "arena" molto semplice, detta anche
 * bump allocator o linear allocator.
 *
 * L'idea e` questa:
 * - si alloca una sola volta un buffer grande;
 * - poi le richieste di memoria non chiamano piu` `malloc`;
 * - si prende il prossimo pezzo libero dentro quel buffer;
 * - un contatore (`offset`) viene spostato in avanti ogni volta.
 *
 * Quindi:
 * - `base`     = inizio del buffer unico;
 * - `capacity` = quanti byte ci sono in totale;
 * - `offset`   = quanti byte abbiamo gia` consumato.
 *
 * La memoria disponibile e` sempre l'intervallo:
 *   [ base + offset, base + capacity )
 */
typedef struct {
    unsigned char *base;
    size_t capacity;
    size_t offset;
} Arena;


/*
 * Tipo di esempio da salvare nell'arena.
 *
 * Non e` speciale: serve solo a mostrare che dall'arena possiamo ottenere
 * memoria per tipi diversi, non solo per byte generici.
 */
typedef struct {
    uint32_t id;
    uint32_t value;
} Record;


/*
 * Anche qui ci serve l'allineamento.
 *
 * Se l'offset corrente non e` compatibile con il tipo richiesto,
 * lo spostiamo al prossimo indirizzo valido.
 *
 * Esempio:
 * se stiamo per allocare un `uint32_t` e l'offset corrente e` 10,
 * dobbiamo spostarci a 12, non possiamo iniziare a 10.
 */
static size_t align_up(size_t value, size_t alignment)
{
    size_t mask = alignment - 1;
    return (value + mask) & ~mask;
}


/*
 * Questa funzione crea l'arena.
 *
 * Qui facciamo l'unica vera `malloc` dell'intero esempio.
 * Dopo questa chiamata, le allocazioni successive useranno tutte lo stesso
 * buffer, senza chiedere nuova memoria al sistema.
 */
static int arena_init(Arena * arena, size_t capacity)
{
    arena->base = malloc(capacity);
    if (!arena->base) {
        return 1;
    }

    arena->capacity = capacity;
    arena->offset = 0;
    memset(arena->base, 0, capacity);
    return 0;
}


/*
 * Distrugge l'arena intera.
 *
 * Questo e` un altro aspetto tipico delle arene:
 * spesso non liberi i singoli oggetti uno per uno,
 * ma liberi tutto in blocco quando hai finito.
 */
static void arena_destroy(Arena * arena)
{
    free(arena->base);
    arena->base = NULL;
    arena->capacity = 0;
    arena->offset = 0;
}


/*
 * Questa e` la funzione che sostituisce concettualmente `malloc`.
 *
 * Leggila cosi`:
 *
 * 1. guardo dove finisce la memoria gia` usata (`arena->offset`);
 * 2. se serve, allineo quel punto;
 * 3. verifico se il nuovo oggetto ci sta ancora nel buffer;
 * 4. se ci sta, restituisco quel pezzo di memoria;
 * 5. aggiorno `offset` per segnare che quello spazio e` ormai occupato.
 *
 * Nota importante:
 * questa funzione non cerca "buchi" liberi nel mezzo,
 * non ricicla singoli oggetti, non tiene metadati complessi.
 * Va solo in avanti.
 *
 * Per questo e` molto veloce, ma anche meno flessibile di `malloc/free`.
 */
static void *arena_alloc(Arena * arena, size_t size, size_t alignment)
{
    size_t start = align_up(arena->offset, alignment);
    size_t end = start + size;

    /*
     * Se `end < start` c'e` stato overflow aritmetico.
     * Se `end > capacity`, l'oggetto non entra nell'arena.
     */
    if (end < start || end > arena->capacity) {
        return NULL;
    }

    /*
     * Aggiorniamo l'offset prima del return:
     * da questo momento in poi quella porzione e` considerata occupata.
     */
    arena->offset = end;
    return arena->base + start;
}


/*
 * Questa operazione e` uno dei motivi per cui le arene sono utili.
 *
 * Invece di fare molte `free`, basta riportare `offset` a zero.
 * Dal punto di vista logico, l'arena torna vuota.
 *
 * Attenzione pero`:
 * i byte vecchi non spariscono magicamente dal buffer.
 * Semplicemente smettiamo di considerarli validi, e le future allocazioni
 * potranno riusarli e sovrascriverli.
 */
static void arena_reset(Arena * arena)
{
    arena->offset = 0;
}

int main(void)
{
    /*
     * In questo demo vogliamo mostrare che dallo stesso buffer possiamo
     * ottenere memoria per oggetti diversi:
     * - un array di interi (`distances`);
     * - un array di struct (`records`);
     * - una stringa (`label`).
     */
    Arena arena;
    uint32_t *distances;
    Record *records;
    char *label;
    size_t i;

    /* Creiamo il buffer unico dell'arena. */
    if (arena_init(&arena, 1024) != 0) {
        fprintf(stderr, "Arena allocation failed.\n");
        return 1;
    }

    /*
     * Queste tre chiamate non fanno tre `malloc` separate.
     *
     * Stanno semplicemente dicendo:
     * - dammi spazio per 8 `uint32_t`;
     * - poi dammi spazio per 3 `Record`;
     * - poi dammi spazio per 32 char.
     *
     * Tutto finisce nello stesso buffer `arena.base`.
     * Gli indirizzi stampati piu` sotto servono proprio a verificarlo.
     */
    distances = arena_alloc(&arena, 8 * sizeof(uint32_t), _Alignof(uint32_t));
    records = arena_alloc(&arena, 3 * sizeof(Record), _Alignof(Record));
    label = arena_alloc(&arena, 32, _Alignof(char));

    if (!distances || !records || !label) {
        fprintf(stderr, "Arena out of memory.\n");
        arena_destroy(&arena);
        return 1;
    }

    /*
     * Da qui in poi usiamo i puntatori come memoria normale.
     * Questo e` importante da capire:
     * una volta ricevuto il puntatore, il codice client non ha quasi bisogno
     * di sapere che quella memoria arriva da un'arena.
     */
    for (i = 0; i < 8; ++i) {
        distances[i] = (uint32_t) (i * 10);
    }

    for (i = 0; i < 3; ++i) {
        records[i].id = (uint32_t) i;
        records[i].value = (uint32_t) (100 + i);
    }

    strcpy(label, "temporary data in arena");

    /*
     * Guardando questi indirizzi puoi vedere la contiguita` "pratica":
     * `distances`, `records` e `label` sono tutti molto vicini
     * perche' sono stati ricavati dallo stesso blocco.
     */
    printf("arena base : %p\n", (void *) arena.base);
    printf("distances  : %p\n", (void *) distances);
    printf("records    : %p\n", (void *) records);
    printf("label      : %p\n", (void *) label);
    printf("used bytes : %zu / %zu\n", arena.offset, arena.capacity);
    printf("label text : %s\n", label);

    /*
     * Dopo il reset non abbiamo liberato un array, poi l'altro, poi la stringa.
     * Abbiamo solo detto: "considera di nuovo tutto il buffer come libero".
     */
    arena_reset(&arena);
    printf("after reset used bytes: %zu / %zu\n", arena.offset, arena.capacity);

    arena_destroy(&arena);
    return 0;
}
