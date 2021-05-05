all: compile

compile:
	gcc -Wall -o main main.c config.c
	gcc -Wall -o cook cook.c config.c
	gcc -Wall -o deliverer deliverer.c config.c

test:
	./main

clean:
	rm -f main cook deliverer