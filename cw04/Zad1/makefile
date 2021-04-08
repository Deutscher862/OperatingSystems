all: compile

compile:
	gcc -Wall main.c -o main
	gcc -Wall exec.c -o exec

test:
	./main ignore null
	./main pending null
	./main mask null
	./main handler null

	./main ignore exec
	./main pending exec
	./main mask exec
	./main handler exec

clean:
	rm main
	rm exec