CONCORD = ./libs/concord

INCLUDE_DIR := $(CONCORD)/include
LIB_DIR := $(CONCORD)/lib

CFLAGS += -I$(INCLUDE_DIR) -Wall -Wpedantic -Wextra
LDFLAGS += -L$(LIB_DIR) $(pkg-config --libs --cflags libcurl) -lcurl

cbalc: %: %.c
	$(CC) $(CFLAGS) -o $@ $< -ldiscord $(LDFLAGS)

clean:
	rm -rf cbalc

.PHONY: clean
