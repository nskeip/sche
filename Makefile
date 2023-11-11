scheme-exe: main.c
	gcc -std=c11 -Wall -Wextra -o scheme-exe ./main.c

.PHONY: debug
debug:
	gcc -g -std=c11 -Wall -Wextra -o scheme-exe ./main.c
	gdb scheme-exe
