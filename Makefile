build:
	@cc -std=c99 -g -Wall prompt.c -ledit -o prompt

run: build
	@./prompt

bear:
	@bear -- make build
