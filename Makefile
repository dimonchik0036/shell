CC=gcc
CFLAGS=-c -Wall
SOURCES=execute.c parse_line.c prompt_line.c shell.c job_control.c command.c job.c builtin.c
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(SOURCES:.c=.h)
EXECUTABLE=myshell


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
