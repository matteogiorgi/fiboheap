CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic


TARGET := binaryheap.o
RUN_TARGET := binaryheap_demo
TEST_TARGET := test_binaryheap
SRC := binaryheap.c
RUN_SRC := binaryheap_demo.c
TEST_SRC := test_binaryheap.c
HEADER := binaryheap.h
.PHONY: all clean run test


all: $(TARGET)

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -c $(SRC) -o $(TARGET)

$(RUN_TARGET): $(SRC) $(RUN_SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(RUN_SRC) -o $(RUN_TARGET)

$(TEST_TARGET): $(SRC) $(TEST_SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o $(TEST_TARGET)

run: $(RUN_TARGET)
	./$(RUN_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(RUN_TARGET) $(TEST_TARGET)
