CC      ?= gcc
CFLAGS  ?= -std=c17 -Wall -Wextra -pedantic -Os
LDFLAGS ?=

SRC     = src/main.c src/elf.c src/dynamic.c src/output.c
OBJ     = $(SRC:.c=.o)
BIN     = neoelf

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(BIN)

install: $(BIN)
	install -Dm755 $(BIN) $(DESTDIR)/usr/bin/$(BIN)

.PHONY: all clean install