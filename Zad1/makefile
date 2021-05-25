all: compile

compile: client.c server.c
	gcc -Wall -o client client.c -lpthread
	gcc -Wall -o server server.c -lpthread

test:
	./server 6666 socket

client1:
	./client Adam local socket

client2:
	./client Tomek local socket

client1_host:
	./client Mateusz xxx 6666

client2_host:
	./client Tomek xxx 6666

clean:
	rm client server socket