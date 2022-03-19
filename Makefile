CC ?= gcc

SRCDIR := src
OBJDIR := obj

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

CFLAGS += -pthread -Wall -Wpedantic -Wextra -I$(SRCDIR)
LDFLAGS += -ldiscord -lcurl

default: cbalc

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

cbalc: $(OBJECTS) $(SRCDIR)/bin/main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f cbalc
	rm -rf $(OBJDIR)

echo:
	@ echo SOURCES: $(SOURCES)
	@ echo OBJECTS: $(OBJECTS)

.PHONY: cbalc clean echo
