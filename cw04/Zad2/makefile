all: compile

compile:
	gcc -Wall main.c -o main

test:
	./main SIGINFO
	./main RESETHAND
	./main NOCLDSTOP

clean:
	rm main