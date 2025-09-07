#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <editline/readline.h>

#include "mpc.h"

#define MAX_ERR_LENGTH 256
#define MAX_SYM_LENGTH 64

// TODO: fix the segfault

typedef struct lval {
  long num;
  char* err;
  char* sym;
  int type;
  int count;
  struct lval** cell;
} lval;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strnlen(m, MAX_ERR_LENGTH) + 1);
  strncpy(v->err, m, MAX_ERR_LENGTH);
  return v;
}

lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strnlen(s, MAX_SYM_LENGTH) + 1);
  strncpy(v->sym, s, MAX_SYM_LENGTH);
  return v;
}

lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval* v) {
  switch (v->type) {
  case LVAL_NUM: break;
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SYM:
    free(v->sym);
    break;
  case LVAL_SEXPR:
    for(int i = 0; i < v->count; i++) {
      lval_del(v->cell[i]);
    }
    free(v->cell);
    break;
  }
  free(v);
}

lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) {
  v->count++;
  // this realloc is going to be _very_ slow
  printf("before realloc: old ptr: %p; new size: %d\n", v->cell, v->count);
  lval** ncell = realloc(v->cell, sizeof(lval*) * v->count);
  if (ncell == NULL) {
    printf("Error reallocating memory!\n");
    // crash here
    return NULL;
  } else {
    v->cell = ncell;
  }
  v->cell[v->count-1] = x;
  return v;
}

lval* lval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  // if root or sexpr, then create empty list
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }

  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    lval_print(v->cell[i]);
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

void lval_print(lval* v) {
  switch (v->type) {
  case LVAL_NUM:
    printf("%li", v->num);
    break;
  case LVAL_ERR:
    printf("Error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_SEXPR:
    lval_expr_print(v, '(', ')');
    break;
  }
}

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}

/* lval eval_op(lval x, char *op, lval y) { */
/*   if (x.type == LVAL_ERR) {return x;} */
/*   if (y.type == LVAL_ERR) {return y;} */

/*   if (strcmp(op, "+") == 0) { */
/*     return lval_num(x.num + y.num); */
/*   } */
/*   if (strcmp(op, "-") == 0) { */
/*     return lval_num(x.num - y.num); */
/*   } */
/*   if (strcmp(op, "*") == 0) { */
/*     return lval_num(x.num * y.num); */
/*   } */
/*   if (strcmp(op, "/") == 0) { */
/*     return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num); */
/*   } */
/*   return lval_err(LERR_BAD_OP); */
/* }; */

lval* eval(mpc_ast_t *t) {
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
  }

  // the operator is always second child
  char *op = t->children[1]->contents;

  // store third child in x
  lval* x = eval(t->children[2]);

  // complete the rest of children
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    /* x = eval_op(x, op, eval(t->children[i])); */
    i++;
  }

  return x;
}

int main(int argc, char **argv) {
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT, " \
  number    : /-?[0-9]+/ ;                            \
  symbol  : '+' | '-' | '*' | '/' ;                   \
  sexpr     : '(' <expr>* ')' ;                       \
  expr      : <number> | <symbol> | <sexpr> ;         \
  lispy     : /^/ <expr>* /$/ ;                       \
            ",
            Number, Symbol, Sexpr, Expr, Lispy);

  puts("Lispy version 0.0.1");

  while (1) {
    char *input = readline("lispy> ");
    add_history(input);

    /* char *input = "+ 2 2"; */

    mpc_result_t r;
    printf("main: parsing stdin\n");
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      printf("main: reading stdin\n");
      lval* x = lval_read(r.output);
      printf("main: printing\n");
      lval_println(x);
      printf("main: freeing stdin\n");
      lval_del(x);
      /* lval result = eval(r.output); */
      /* lval_println(result); */
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

  return 0;
}
