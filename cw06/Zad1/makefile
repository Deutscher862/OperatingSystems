all: compile

compile:
	gcc -Wall server.c -o server
	gcc -Wall client.c -o client

test:
	@echo "[TEST]"
	@./main test.txt

clean:
	rm server
	rm client