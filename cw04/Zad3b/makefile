all: compile

compile:
	gcc -Wall catcher.c -o catcher
	gcc -Wall sender.c -o sender

test:
	@echo "[TEST] KILL"
	@./catcher KILL&
	@./sender `pidof catcher` 100 KILL

	@echo "[TEST] SIGQUEUE"
	@./catcher SIGQUEUE&
	@./sender `pidof catcher` 100 SIGQUEUE

	@echo "[TEST] SIGRT"
	@./catcher SIGRT&
	@./sender `pidof catcher` 100 SIGRT

clean:
	rm catcher
	rm sender