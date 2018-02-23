CC=gcc
CFLAGS=-c -Wall
HEADERS=shell.h
SOURCES=execute.c parse_line.c prompt_line.c shell.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=myshell


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
