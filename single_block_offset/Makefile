CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic

TARGET := single_block_demo
SRC := single_block_demo.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
