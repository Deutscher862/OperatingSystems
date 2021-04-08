all: mkdir compile

mkdir:
	mkdir output

compile: main.o my_library.o
	gcc -Wall output/main.o output/my_library.o -o main

main.o:
	gcc -c -Wall main.c -o output/main.o

my_library.o:
	gcc -c -Wall my_library.c -o output/my_library.o

clean:
	rm *o main
