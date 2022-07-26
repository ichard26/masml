bin := masml
baseFlags := -std=c11 -lm -Wall -Wextra -Wconversion -pedantic

build-debug-asan: masml.c
	@clang masml.c -o $(bin) -g $(baseFlags) \
		-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero \
		-fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment $(CFLAGS)

build-release: masml.c
	@clang masml.c -o $(bin) -g0 $(baseFlags) -O3 $(CFLAGS)
