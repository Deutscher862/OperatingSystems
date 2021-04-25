all: compile

compile:
	gcc -Wall server.c -o server
	gcc -Wall client.c -o client

test:
	@echo "[TEST]"
	@./server
	@./client

clean:
	rm server
	rm client