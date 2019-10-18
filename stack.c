#include "lisp.h"

#define STACK_SIZE 100

static Pair *stack[STACK_SIZE];
static int stack_index = 0;
int cons_pointers = 0;

void save(Pair *p)
{
  stack[stack_index++] = p;
}

void forget(void)
{
  --stack_index;
}

