default: all

CFLAGS := -I./include -g --std=gnu99
CC := gcc

BINARIES := ptu-c46-ach
all : $(BINARIES)

LIBS := -lach -lrt  -lncurses 

ptu-c46-ach: src/ptu-c46-ach.o
	gcc -o $@ $< $(LIBS) -lm -lc

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) src/*.o

