CFLAGS = -Wall -pedantic -g

.PHONY: all
all: mysh test

mysh: mysh.c
	gcc $(CFLAGS) -o $@ $^

test: test.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f mysh test
