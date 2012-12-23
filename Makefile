CFLAGS=-c -g
SOURCES=main.c parser.c scanner.c semantic.c generator.c
OBJECTS=$(SOURCES:.c=.o)
DEPS=semantic.h scanner.h token.h parser.h generator.h
CC=gcc
EXECUTABLE=comp

all: $(SOURCES) $(EXECUTABLE) -lm $(DEPS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE) $(OBJECTS)
