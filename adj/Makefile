CC := gcc
CFLAGS := -O0 -std=c11 -Wall -Wextra -pedantic


TARGET := adj_graph.o
TEST_TARGET := test_dimacs_adj
BENCH_TARGET := bench_adj
SRC := adj_graph.c
TEST_SRC := test_dimacs_adj.c
BENCH_SRC := bench_adj.c
HEADER := adj_graph.h
.PHONY: all clean run test


all: $(TARGET)

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -c $(SRC) -o $(TARGET)

$(TEST_TARGET): $(SRC) $(TEST_SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o $(TEST_TARGET)

$(BENCH_TARGET): $(SRC) $(BENCH_TARGET) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(BENCH_SRC) -o $(BENCH_TARGET)

test1: $(TEST_TARGET)
	./$(TEST_TARGET) USA-road-d.FLA.gr

test2: $(TEST_TARGET)
	./$(TEST_TARGET) USA-road-d.USA.gr

bench1: $(BENCH_TARGET)
	./$(BENCH_TARGET) USA-road-d.FLA.gr

bench2: $(BENCH_TARGET)
	./$(BENCH_TARGET) USA-road-d.USA.gr

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(BENCH_TARGET)
