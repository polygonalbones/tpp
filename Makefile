BUILD_DIR = build

CC = clang
CFLAGS = \
	-Wall \
	-Wextra \
	-Wswitch \
	-Werror=switch \
	-std=c99 \
	-Iinclude \
	-Ibuild \
	-MMD \
	-MP

TARGET = $(BUILD_DIR)/tpp

SRC = \
	src/main.c \
	src/libtpp.c

OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEP)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
