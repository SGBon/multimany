CC=gcc
CFLAGS=-c -Wall -Wextra -O2
LIBS=-lOpenCL # put libraries here
LDFLAGS=$(LIBS)
SOURCES=hello.c # put source files here
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
