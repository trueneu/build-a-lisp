build:
	@cc -std=c99 -g -Wall lispy.c mpc.c -ledit -lm -o lispy

run: build
	@./lispy

bear:
	@bear -- make build
