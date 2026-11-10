CC = clang
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

BUILD_DIR = build
TARGET = $(BUILD_DIR)/tpp
SRC = src/main.c
OBJ = $(addprefix build/, $(notdir $(SRC:.c=.o)))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

vpath %.c src

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all clean
