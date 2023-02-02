CC := clang
BASE_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic -Iclikit -MMD

vpath %.c src
SRC := masml.c util.c
OBJ := $(SRC:.c=.o) clikit.a
BIN := masml

DIR := build
DEBUG_DIR := $(DIR)/debug
REL_DIR := $(DIR)/release

REL_OBJ := $(foreach o,$(OBJ),$(REL_DIR)/$(o))
DEBUG_OBJ := $(foreach o,$(OBJ),$(DEBUG_DIR)/$(o))
DEPS := $(patsubst %.o,%.d,$(filter %.o,$(REL_OBJ) $(DEBUG_OBJ)))

build-debug: CFLAGS := -g -fsanitize=address -fsanitize=undefined \
					-fno-sanitize-recover=all -fsanitize=float-divide-by-zero \
					-fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
build-release: CFLAGS := -g0 -O3
export CFLAGS

$(DEBUG_DIR)/%.o $(REL_DIR)/%.o: %.c | setup-build
	$(CC) -c $< -o $@ $(BASE_FLAGS) $(CFLAGS)

.PHONY: clean setup-build build-debug build-release

build-debug: $(DEBUG_OBJ)
	$(CC) $^ -o $(BIN) $(BASE_FLAGS) -lm $(CFLAGS)

build-release: $(REL_OBJ)
	$(CC) $^ -o $(BIN) $(BASE_FLAGS) -lm $(CFLAGS)

%/clikit.a: setup-build
	$(MAKE) -C clikit CC=$(CC) OUT=../clikit.a DIR=$(realpath $(dir $@))/clikit

setup-build:
	@mkdir -p $(DEBUG_DIR) $(REL_DIR) || :

clean:
	rm $(DIR) -r || :

-include $(DEPS)
