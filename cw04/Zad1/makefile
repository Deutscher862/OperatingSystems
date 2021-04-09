all: compile

compile:
	gcc -Wall main.c -o main
	gcc -Wall exec.c -o exec

test:
	@echo "[TEST] ignore"
	@./main ignore null
	@echo "[TEST] pending"
	@./main pending null
	@echo "[TEST] mask"
	@./main mask null
	@echo "[TEST] handler"
	@./main handler null

	@echo "[TEST] ignore exec"
	@./main ignore exec
	@echo "[TEST] pending exec"
	@./main pending exec
	@echo "[TEST] mask exec"
	@./main mask exec

clean:
	rm main
	rm exec