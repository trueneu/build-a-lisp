#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#define INPUT_BUFFER_LENGTH 2048

int main(int argc, char **argv) {
  puts("Lispy version 0.0.1");

  while (1) {
    char* input = readline("lispy> ");
    add_history(input);
    printf("No you're a %s\n", input);
    free(input);
  }

  return 0;
}
