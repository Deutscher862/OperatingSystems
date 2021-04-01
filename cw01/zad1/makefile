all: mkdir static shared

static:
	gcc -Wall -c my_library.c -o output/my_library_static.o
	ar rcs output/libmy_library.a output/my_library_static.o

shared:
	gcc -Wall -fPIC -c my_library.c -o output/my_library_shared.o
	gcc -Wall -shared -Wl,-soname,libmy_library.so.1 -o output/libmy_library.so.1.0.1 output/my_library_shared.o -lc

mkdir:
	mkdir output

clean:
	rm -R output