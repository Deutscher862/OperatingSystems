all: compile

compile:
	gcc -Wall main.c -o main

test:
	@echo "[TEST] SIGINFO"
	@./main SIGINFO
	@echo "[TEST] RESETHAND"
	@./main RESETHAND
	@echo "[TEST] NOCLDSTOP"
	@./main NOCLDSTOP

clean:
	rm main