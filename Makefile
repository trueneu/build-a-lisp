build:
	@cc -std=c99 -g -Wall parsing.c mpc.c -ledit -lm -o lispy

run: build
	@./lispy

bear:
	@bear -- make build
