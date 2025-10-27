CC = gcc

CFLAGS = -O3 -Wall -Iinclude
LDFLAGS = -lraylib

EXE = out/a.out
SRC = $(shell find src -name '*.c')
OBJ = $(patsubst src/%.c, out/%.o, $(SRC))

exe: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $(EXE)

out/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf out
