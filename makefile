CFLAGS = -Wall -Werror 
LIBS = -lpthread

all: build

build: s-talk

s-talk: s-talk.c list.h list.c
	gcc $(CFLAGS) s-talk.c list.c -o s-talk $(LIBS)
	
clean:
	rm -f s-talk
