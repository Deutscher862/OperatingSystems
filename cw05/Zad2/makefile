all: compile

compile:
	gcc -Wall main.c -o main

test:
	@echo "[TEST SortBy Date]"
	@./main date

	@echo "[TEST SortBy Sender]"
	@./main sender

	@echo "[TEST Send]"
	@./main test@test.pl Hello Hello

clean:
	rm main