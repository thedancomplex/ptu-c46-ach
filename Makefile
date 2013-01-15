default: all

CFLAGS := -I./include -g --std=gnu99
CXXFLAGS := -I./include -g

CC := gcc
CXX := g++

BINARIES := ptu-c46-ach
all : $(BINARIES)

LIBS := -lach -lrt -lreadline

ptu-c46-ach: src/ptu-c46-ach.o
	$(CXX) -o $@ $< $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARIES) src/*.o

