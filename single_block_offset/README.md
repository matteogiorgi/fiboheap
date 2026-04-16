# Single Block With Manual Offsets

Questo esempio alloca un solo buffer con `malloc` e poi costruisce a mano il layout interno:

- `NodeHeader[node_count]`
- `uint32_t offsets[node_count + 1]`
- `uint32_t edges[edge_count]`

L'accesso alle varie sezioni avviene calcolando gli offset e rispettando l'allineamento con `align_up`.

Per eseguirlo:

```sh
make run
```
