all:
	gcc -Wall -pthread -o bin/shell src/main.c

debug:
	gcc -g -Wall -pthread -o bin/shell src/main.c

clean:
	rm bin/*

.PHONY: all clean
