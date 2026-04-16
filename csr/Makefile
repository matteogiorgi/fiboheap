# Compilatore C usato per tutte le build.
CC := pcc

# Flag di compilazione:
# -O0        -> disabilita le ottimizzazioni.
#  				(utile per debug e analisi del comportamento)
# -std=c11   -> richiede la conformità allo standard C11.
# -Wall      -> abilita il set principale di warning del compilatore.
# -Wextra    -> abilita warning aggiuntivi oltre a quelli di -Wall.
# -pedantic  -> segnala costrutti non strettamente conformi allo standard scelto.
CFLAGS := -O0 -std=c11




# Nome del file oggetto generato compilando
# il modulo principale senza linking finale.
TARGET := csr_graph.o

# Nome dell'eseguibile che contiene
# il test del parser/formato DIMACS.
TEST_TARGET := test_dimacs

# Nome dell'eseguibile usato per benchmark o prove prestazionali.
BENCH_TARGET := bench_csr

# Sorgente principale che implementa la struttura CSR e le relative funzioni.
SRC := csr_graph.c

# Sorgente del programma di test.
TEST_SRC := test_dimacs.c

# Sorgente del programma di benchmark.
BENCH_SRC := bench_csr.c

# Header condiviso, incluso dai sorgenti
# che usano l'interfaccia del grafo CSR.
HEADER := csr_graph.h

# Elenco di target "fittizi": non corrispondono a
# file reali e vanno sempre eseguiti quando richiesti.
.PHONY: all clean run test




# Target di default: costruisce il file oggetto principale.
all: $(TARGET)


$(TARGET): $(SRC) $(HEADER)
	# Compila soltanto $(SRC) in un file oggetto:
	# $(CC)       -> invoca gcc.
	# $(CFLAGS)   -> applica i flag definiti sopra.
	# -c          -> esegue la sola compilazione senza fase di linking.
	# $(SRC)      -> file sorgente da compilare.
	# -o $(TARGET): nome del file oggetto prodotto in uscita.
	$(CC) $(CFLAGS) -c $(SRC) -o $(TARGET)


$(TEST_TARGET): $(SRC) $(TEST_SRC) $(HEADER)
	# Compila e collega il codice sorgente principale insieme al target di test,
	# producendo l'eseguibile finale $(TEST_TARGET).
	# In un singolo comando gcc:
	# $(SRC)                -> fornisce l'implementazione del modulo CSR.
	# $(TEST_SRC)           -> contiene il main del programma di test.
	# -o $(TEST_TARGET)     -> assegna questo nome all'eseguibile generato.
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o $(TEST_TARGET)


$(BENCH_TARGET): $(SRC) $(BENCH_SRC) $(HEADER)
	# Compila e collega il codice sorgente principale insieme al target di benchmark,
	# producendo l'eseguibile finale $(BENCH_TARGET).
	# Anche qui gcc riceve:
	# $(SRC)                 -> implementazione condivisa.
	# $(BENCH_SRC)           -> contiene il main del programma di benchmark.
	# -o $(BENCH_TARGET)     -> nome dell'eseguibile generato.
	$(CC) $(CFLAGS) $(SRC) $(BENCH_SRC) -o $(BENCH_TARGET)


test1: $(TEST_TARGET)
	# Esegue il programma di test sul dataset Florida.
	# ./$(TEST_TARGET)       -> lancia l'eseguibile appena costruito nella directory corrente.
	# USA-road-d.FLA.gr      -> file grafo passato come primo argomento da riga di comando.
	./$(TEST_TARGET) USA-road-d.FLA.gr


test2: $(TEST_TARGET)
	# Esegue lo stesso test sul dataset USA completo.
	./$(TEST_TARGET) USA-road-d.USA.gr


bench1: $(BENCH_TARGET)
	# Avvia il benchmark sul dataset Florida.
	./$(BENCH_TARGET) USA-road-d.FLA.gr


bench2: $(BENCH_TARGET)
	# Avvia il benchmark sul dataset USA completo passando anche il valore aggiuntivo 12345,
	# presumibilmente usato dal programma come parametro numerico di configurazione.
	./$(BENCH_TARGET) USA-road-d.USA.gr 12345


clean:
	# Rimuove forzatamente gli artefatti generati dalla build:
	# -f                    -> evita errori se i file non esistono.
	# $(TARGET)             -> file oggetto principale.
	# $(TEST_TARGET)        -> eseguibile dei test.
	# $(BENCH_TARGET)       -> eseguibile di benchmark.
	rm -f $(TARGET) $(TEST_TARGET) $(BENCH_TARGET)
