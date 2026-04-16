# Arena / Bump Allocator

Questo esempio usa un buffer unico e un offset crescente:

- `arena_init` crea il buffer;
- `arena_alloc` restituisce sottoblocchi allineati;
- `arena_reset` azzera solo l'offset e rende riutilizzabile tutta l'arena.

Il vantaggio è che le allocazioni sono molto veloci e tutte vicine in memoria. Il limite è che non puoi liberare un singolo oggetto in mezzo: o resetti tutta l'arena, o la distruggi.

Per eseguirlo:

```sh
make run
```
