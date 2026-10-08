CC         ?= cc
CFLAGS     ?= -Wall -O2
LDFLAGS    = -static
TARGET     = bname
SRC        = bname.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
