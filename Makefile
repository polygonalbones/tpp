BUILD_DIR = build

CC = clang
CFLAGS = -Wall -Wextra -Wswitch -Werror=switch -std=c99 -Iinclude -Ibuild

TBGEN_TARGET = $(BUILD_DIR)/tablegen
TBGEN_SRC = \
	src/murmur3.c \
	tools/tablegen.c

KEYWORDS_H = $(BUILD_DIR)/keywords.h

TARGET = $(BUILD_DIR)/tpp
SRC = \
	src/main.c \
	src/lexer.c \
	src/murmur3.c

HEADERS = \
	include/tppalloc.h \
	include/lexer.h \
	include/murmur3.h

OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)

all: $(KEYWORDS_H) $(TARGET)

$(KEYWORDS_H): $(TBGEN_TARGET) include/keywords.txt
	$(TBGEN_TARGET)

$(TBGEN_TARGET): $(TBGEN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TBGEN_SRC)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all clean
