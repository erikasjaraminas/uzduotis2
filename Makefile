CC = gcc
BIN := programa
SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)

CPPFLAGS := -I$(shell pwd)
CFLAGS := -pthread

.PHONY: all clean install

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BIN) $(OBJ)

install:
	install -m 666 watchlist.conf /etc/watchlist.conf
	install -m 666 daemon.log /var/log/daemon.log
