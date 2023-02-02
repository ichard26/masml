# Referenced resources:
# - https://stackoverflow.com/a/1852977/18155471

BASE_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic -MMD
OUT := clikit.a

SRC := clikit.c
OBJ := $(foreach o,$(SRC:.c=.o),$(DIR)/$(o))

DIR := build

.PHONY: setup-build clean

$(DIR)/$(OUT): $(OBJ)
	ar rcs $@ $^

setup-build:
	@mkdir -p $(DIR) || :

clean:
	rm -r $(DIR) || :

$(DIR)/%.o: %.c | setup-build
	$(CC) -c $< -o $@ $(BASE_FLAGS) $(CFLAGS)

-include $(OBJ:.o=.d)
