sche: main.c
	gcc -std=c11 -Wall -Wextra -o scheme-exe ./main.c

.PHONY: run
run: sche
	./scheme-exe
