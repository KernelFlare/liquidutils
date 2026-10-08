CC       ?= cc
CFLAGS   ?= -Wall -Wextra -O2 -std=c99 -pedantic
LDFLAGS  ?= -static
PREFIX   ?= /usr/local
TARGET    = dsh
SRC       = dsh.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

install: $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install clean
