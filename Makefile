# ------------------------------------------------------------
#  Project:  C Utils (cstr & vector)
#  Build system: GNU Make (Unix‑like only)
# ------------------------------------------------------------

# ── General tools & flags ─────────────────────────────────────
CC      := gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g -I$(INC_DIR)
LDFLAGS := -g

# ── Directory variables ─────────────────────────────────────
SRC_DIR   := src
INC_DIR   := include
TEST_DIR  := test
OBJ_DIR   := obj
LIB_DIR   := lib
BIN_DIR   := bin

# ── Library names ────────────────────────────────────────────
STATIC_LIB := $(LIB_DIR)/libcutils.a
SHARED_LIB := $(LIB_DIR)/libcutils.so

# ── Source & object files ───────────────────────────────────
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# ── Test programs ─────────────────────────────────────────────
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRCS))

# ── Default target ───────────────────────────────────────────
.PHONY: all
all: $(STATIC_LIB) $(SHARED_LIB) $(TEST_BINS)

# ── Create output directories as needed ─────────────────────
$(OBJ_DIR):
	@mkdir -p $@

$(LIB_DIR):
	@mkdir -p $@

$(BIN_DIR):
	@mkdir -p $@

# ── Build object files (used for both libs) ─────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# ── Static library ────────────────────────────────────────────
$(STATIC_LIB): $(OBJ_FILES) | $(LIB_DIR)
	@rm -f $@
	$(AR) rcs $@ $(OBJ_FILES)

# ── Shared library ───────────────────────────────────────────
$(SHARED_LIB): $(OBJ_FILES) | $(LIB_DIR)
	@rm -f $@
	$(CC) -shared -o $@ $(OBJ_FILES) $(LDFLAGS)

# ── Build each test executable (statically linked) ────────────
$(BIN_DIR)/%: $(TEST_DIR)/%.c $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(LIB_DIR)/libcutils.a -o $@

# ── Clean target ─────────────────────────────────────────────
.PHONY: clean
clean:
	@echo "Removing build artefacts..."
	@rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

# ── Convenience targets ─────────────────────────────────────
.PHONY: debug
debug: all    # `all` already uses `-g`; kept for explicitness

.PHONY: reinstall
reinstall: clean all
