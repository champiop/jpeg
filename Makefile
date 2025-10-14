# ==== Configuration ====
CC       := gcc
CFLAGS   := -std=c11 -Wall -Wextra -O0

# ==== Directories ====
SRC_DIR      := src
BUILD_DIR    := build
BIN_DIR      := $(BUILD_DIR)/bin
INCLUDE_DIR  := include
LIB_DIR      := lib

# ==== Files ====
SRC        := $(SRC_DIR)/jpeg.c
OBJ        := $(BUILD_DIR)/jpeg.o
EXE_NAME   := $(BIN_DIR)/jpeg
LIB        := $(LIB_DIR)/libnetpbm.a

# ==== Default target ====
all: $(EXE_NAME)

# ==== Rule: build executable ====
$(EXE_NAME): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

# ==== Compile C sources ====
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_DIR)

# ==== Directory creation ====
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# ==== Clean ====
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
