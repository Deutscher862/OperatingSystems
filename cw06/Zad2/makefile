all: compile

compile:
	gcc -Wall server.c -o server -lrt
	gcc -Wall client.c -o client -lrt

test:
	@echo "[TEST]"
	@./server

clean:
	rm server
	rm client