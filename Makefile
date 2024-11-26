CC = gcc
CFLAGS = -c -Wall -g
LDFLAGS = -ljpeg
SOURCES = mandel.c jpegrw.c mandelmovie.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLES = mandel mandelmovie

all: $(EXECUTABLES)

# Build mandel
mandel: mandel.o jpegrw.o
	$(CC) mandel.o jpegrw.o $(LDFLAGS) -o mandel

# Build mandelmovie
mandelmovie: mandelmovie.o jpegrw.o
	$(CC) mandelmovie.o jpegrw.o $(LDFLAGS) -o mandelmovie

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	$(CC) -MM $< > $*.d

clean:
	rm -rf $(OBJECTS) $(EXECUTABLES) *.d *.jpg *.mpg

