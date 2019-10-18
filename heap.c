#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

#define HEAP_SIZE 1000

static Pair memory1[HEAP_SIZE], memory2[HEAP_SIZE];
static Pair *free_ptr = memory1;
static void swap_memory(void);

// For GC, need to load pointers to Elements in all registers into the
// working memory in a list structure that will be traversed.

// Which registers need to be preserved though?

char *string_alloc(int n)
{
  return (char *) malloc(n + 1);
}

// When not full, just the next element in array. Otherwise need to garbage
// collect.
Pair *get_next_free_ptr(void)
{
  Pair *p = free_ptr;

  if (free_ptr + 1 >= memory1 + HEAP_SIZE)
    swap_memory();

  free_ptr += 1;
  return p;
}

void swap_memory(void)
{
  for (int i = 0; i < stack_index; ++i) {
    copy(stack[stack_index].contents.pair_ptr);
  }
}

void copy(Pair *p)
{
  if (p && p->car.type_tag != BROKEN_HEART) {
    Pair *q = get_next_free_ptr();

    *q = *p;

    p->car.type_tag = BROKEN_HEART;
    p->cdr.contents.pair_ptr = q;

    copy(q->car);
    copy(q->cdr);
  }
}