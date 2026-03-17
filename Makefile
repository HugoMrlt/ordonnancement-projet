CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SRC_DIR = src
BUILD_DIR = build

SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/heuristiques.c $(SRC_DIR)/vns.c $(SRC_DIR)/branch_bound.c
TARGET = $(BUILD_DIR)/openshop

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
