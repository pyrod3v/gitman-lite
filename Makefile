CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -Wextra -O2
LIBS = -lcurl

SRC = src
BIN = bin

SRCS = $(SRC)/main.c $(SRC)/gitman.c
OBJS = $(SRCS:.c=.o)

TARGET = $(BIN)/gm

all: $(TARGET)

# Link the object files
$(TARGET): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Build object files
$(SRC)/%.o: $(SRC)/%.c $(SRC)/gitman.h | $(BIN)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN):
	mkdir -p $(BIN)

clean:
	rm -f $(OBJS) $(TARGET)
	rmdir --ignore-fail-on-non-empty $(BIN)

.PHONY: all clean
