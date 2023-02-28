CC = gcc
EXEC = main

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

CFLAGS = -std=c11 -g -pedantic -Werror -Wall -Wconversion $(foreach D, $(INCLUDE_DIR), -I$(D))
LDFLAGS = -lm

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm $(BUILD_DIR)/*.o $(EXEC)

