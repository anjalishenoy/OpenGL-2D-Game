CC = g++
CFLAGS = -Wall
PROG = game

SRCS = main.cpp imageloader.cpp text3d.cpp
LIBS = -lglut -lGL -lGLU 

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)

