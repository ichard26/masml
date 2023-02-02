CC := clang
BASE_FLAGS := -g -std=c11 -Wall -Wextra -Wconversion -pedantic -Iclikit

vpath %.c src
SRC := masml.c util.c
OBJ := $(SRC:.c=.o) clikit.a
BIN := masml

DIR := build
DEBUG_DIR := $(DIR)/debug
REL_DIR := $(DIR)/release

build-debug-asan: CFLAGS := -fsanitize=address -fsanitize=undefined \
					-fno-sanitize-recover=all -fsanitize=float-divide-by-zero \
					-fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
build-release: CFLAGS := -g0 -O3
export CFLAGS

$(DEBUG_DIR)/%.o $(REL_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(BASE_FLAGS) $(CFLAGS)

.PHONY: clean setup-build build-debug-asan build-release

build-debug-asan: setup-build $(foreach o,$(OBJ),$(DEBUG_DIR)/$(o))
	$(CC) $(filter %.o %.a,$^) -o $(BIN) $(BASE_FLAGS) -lm $(CFLAGS)

build-release: setup-build $(foreach o,$(OBJ),$(REL_DIR)/$(o))
	$(CC) $(filter %.o %.a,$^) -o $(BIN) $(BASE_FLAGS) -lm $(CFLAGS)

%/clikit.a: setup-build
	$(MAKE) -C clikit CC=$(CC) OUT=../$(notdir $@) DIR=$(realpath $(dir $@))/clikit

setup-build:
	@mkdir -p $(DEBUG_DIR) $(REL_DIR) || :

clean:
	rm $(DIR) -r || :
