CC := clang
BASE_FLAGS := -g -std=c11 -Wall -Wextra -Wconversion -pedantic
BIN := masml

SRC := masml.c clikit.c
OBJ := $(SRC:.c=.o)

DIR := build
DEBUG_DIR := $(DIR)/debug
REL_DIR := $(DIR)/release

build-debug-asan: BASE_FLAGS += -fsanitize=address -fsanitize=undefined \
					-fno-sanitize-recover=all -fsanitize=float-divide-by-zero \
					-fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
build-release: BASE_FLAGS += -g0 -O3

$(DEBUG_DIR)/%.o $(REL_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(BASE_FLAGS) $(CFLAGS)

.PHONY: clean setup-build build-debug-asan build-release

build-debug-asan: setup-build $(foreach o,$(OBJ),$(DEBUG_DIR)/$(o))
	$(CC) $(filter %.o,$^) -o $(BIN) -lm $(BASE_FLAGS) $(CFLAGS)

build-release: setup-build $(foreach o,$(OBJ),$(REL_DIR)/$(o))
	$(CC) $(filter %.o,$^) -o $(BIN) -lm $(BASE_FLAGS) $(CFLAGS)

setup-build:
	@mkdir -p $(DEBUG_DIR) $(REL_DIR) || :

clean:
	rm $(DIR) -r || :
