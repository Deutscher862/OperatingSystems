all: build example_test clean

build: main.c
	gcc -Wall -o main main.c -lpthread -lm

test: main
	./main 4 sign mountain.ascii.pgm output_sign
	./main 4 block mountain.ascii.pgm output_block

clean:
	rm -f main output_sign output_block