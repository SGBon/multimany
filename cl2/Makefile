CC=gcc
CFLAGS=-c -Wall -Wextra -g3 -std=c99
LIBS=-lOpenCL # put libraries here
LDFLAGS=$(LIBS)
SOURCES=multi.c clutil.c # put source files here
OBJECTS=$(SOURCES:.c=.o)
DEPEND=.depend
EXECUTABLE=a.out

all: $(SOURCES) $(EXECUTABLE) $(DEPEND)

$(DEPEND): $(SOURCES)
	rm -f $@
	$(CC) $(CFLAGS) -MM $^ > $@;

include .depend

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean run
run: all
	./$(EXECUTABLE)
clean:
	rm $(OBJECTS) $(EXECUTABLE) $(DEPEND)
