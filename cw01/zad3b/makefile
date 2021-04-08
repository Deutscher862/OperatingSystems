optimalization_level = 3;

all: make_lib

make_lib: dirs main_static main_shared main_dynamic

dirs:
	mkdir -p output

static_lib:
	gcc -Wall -c my_library.c -o output/my_library.o
	ar rcs output/my_library.a output/my_library.o

shared_lib:
	gcc -Wall -c -fPIC my_library.c -o output/my_library.o
	gcc -Wall -shared -fPIC -o libdiff.so output/my_library.o

main_static: static_lib 
	gcc -Wall -c main.c -o output/main.o -O$(optimalization_level)
	gcc -Wall output/main.o output/my_library.a -o main.out

main_shared: shared_lib
	gcc -Wall main.c -L. -o main.out -ldiff -Wl,-rpath=. -O$(optimalization_level)

main_dynamic: shared_lib
	gcc -Wall main.c -o main.out -ldl -D dynamic -O$(optimalization_level)

clean:
	rm *o main
