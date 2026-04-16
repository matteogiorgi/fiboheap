# FiboHeap

Questo repository contiene un piccolo progetto in C dedicato allo studio e all'implementazione di una `binary heap` generica.

Il lavoro parte da un'analisi dei sorgenti di `glibc` per verificare se esista gia' una struttura riusabile equivalente a una priority queue o a una heap binaria; da questa analisi deriva poi l'implementazione locale della libreria.

> Symlink ai sorgenti della libreria GNU C: `glibc-2.36 -> /usr/src/glibc/glibc-2.36`

Nel repository sono presenti:

- `binaryheap.h`: interfaccia pubblica della libreria;
- `binaryheap.c`: implementazione della min-heap generica basata su `void **` e funzione comparatore;
- `binaryheap_demo.c`: programma dimostrativo di utilizzo;
- `test_binaryheap.c`: test automatici con `assert`;
- `Makefile`: build dell'oggetto, demo ed esecuzione dei test;
- `report/report.tex` e `report/report.pdf`: relazione tecnica del lavoro svolto.




## Testing

Per testare il progetto, l'implementazione degli heap verrà usata per eseguire algoritmi di esplorazione come Dijkstra, Prim, A\*, usando le [USA road networks](https://www.diag.uniroma1.it//challenge9/download.shtml) fornite da [DIMACS](http://dimacs.rutgers.edu/).




## Comandi utili:

- `make`: compila `binaryheap.o`
- `make run`: compila ed esegue la demo
- `make test`: compila ed esegue i test
- `make clean`: rimuove gli artefatti di build
