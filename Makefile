BUILD_DIR = build

CC = clang
CFLAGS = -Wall -Wextra -Wswitch -Werror=switch -std=c99 -Iinclude

TARGET = $(BUILD_DIR)/tpp
SRC = src/main.c src/lexer.c src/hmap.c src/hmap_err.c
HEADERS = include/tppalloc.h include/lexer.h \
					include/hmap.h include/hmap_err.h
OBJ = $(addprefix build/, $(notdir $(SRC:.c=.o)))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

vpath %.c src

$(BUILD_DIR)/%.o: %.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all clean
