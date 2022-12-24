# Referenced resources:
# - https://stackoverflow.com/a/1852977/18155471

BASE_FLAGS := -g -std=c11 -Wall -Wextra -Wconversion -pedantic
OUT := clikit.a

SRC := clikit.c
OBJ := $(SRC:.c=.o)

DIR := build

.PHONY: setup-build build

build: setup-build $(foreach o,$(OBJ),$(DIR)/$(o))
	ar rcs $(DIR)/$(OUT) $(filter %.o,$^)

setup-build:
	@mkdir -p $(DIR) || :

clean:
	rm -r $(DIR) || :

$(DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(BASE_FLAGS) $(CFLAGS)
