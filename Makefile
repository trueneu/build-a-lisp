build:
	@cc -std=c99 -g -Wall lispy.c mpc.c -lc -ledit -lm -o lispy

buildref:
	@cc -std=c99 -g -Wall reference.c mpc.c -lc -ledit -lm -o reference

run: build
	@./lispy

bear:
	@bear -- make build
