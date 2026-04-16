CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -pedantic

TARGET := arena_demo
SRC := arena_demo.c

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
