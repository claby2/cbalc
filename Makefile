CC ?= gcc

CFLAGS += -pthread -Wall -Wpedantic -Wextra
LDFLAGS += -ldiscord -lcurl

cbalc: %: %.c
	$(CC) $(CFLAGS) -o $@ $< -ldiscord $(LDFLAGS)

clean:
	rm -rf cbalc

.PHONY: clean
