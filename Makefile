CC = gcc
CFLAGS = -lncurses
OUT = pbsh
FILE = pbshell
MAIN = main

all: compile run

compile: $(FILE).c $(MAIN).c
	$(CC) -o $(OUT) $(FILE).c $(CFLAGS)
	$(CC) -o $(MAIN) $(MAIN).c $(CFLAGS) 

run:
	./$(MAIN)

clean:
	rm $(OUT) $(MAIN)
