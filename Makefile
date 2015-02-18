all:
	gcc -Wall -o bin/shell src/main.c

debug:
	gcc -g -Wall -o bin/shell src/main.c

clean:
	rm bin/*

.PHONY: all clean
