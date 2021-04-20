all:
	gcc -Wall main.c -o main
	gcc -Wall producer.c -o producer
	gcc -Wall consumer.c -o consumer

test:
	@echo "[TEST]"
	./main
	rm fifo
	rm tmp.txt
clean:
	rm main
	rm consumer
	rm producer