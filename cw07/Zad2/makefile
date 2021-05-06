all: compile

compile:
	gcc -Wall -o main main.c config.c -lpthread -lrt
	gcc -Wall -o cook cook.c config.c -lpthread -lrt
	gcc -Wall -o deliverer deliverer.c config.c -lpthread -lrt

test:
	./main

clean:
	rm -f main cook deliverer