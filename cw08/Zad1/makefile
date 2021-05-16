all: compile test

compile: main.c
	gcc -Wall -o main main.c -lpthread -lm

test: main
	./main 1 sign pepper.ascii.pgm sign_result.ascii.pgm
	./main 2 sign pepper.ascii.pgm sign_result.ascii.pgm
	./main 4 sign pepper.ascii.pgm sign_result.ascii.pgm
	./main 8 sign pepper.ascii.pgm sign_result.ascii.pgm
	./main 16 sign pepper.ascii.pgm sign_result.ascii.pgm
	
	./main 1 block pepper.ascii.pgm block_result.ascii.pmg
	./main 2 block pepper.ascii.pgm block_result.ascii.pmg
	./main 4 block pepper.ascii.pgm block_result.ascii.pmg
	./main 8 block pepper.ascii.pgm block_result.ascii.pmg
	./main 16 block pepper.ascii.pgm block_result.ascii.pmg

clean:
	rm main
	rm sign_result.ascii.pgm
	rm block_result.ascii.pmg