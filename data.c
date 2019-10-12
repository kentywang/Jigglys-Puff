#include <string.h>
#include "lisp.h"

/*
Should return Element for closure property.

If the elements to cons are pairs, we want to the cons to hold their
references so if their contents change, the cons does too. But if the
elements are atoms (strings, symbols, numbers, boolean), we want a copy
of them.
*/
Element make_cons(const Element x, const Element y)
{
  Element e;

  Pair *p = get_next_free_ptr();

  p->car = clone(x);
  p->cdr = clone(y);

  e.type_tag = PAIR;
  e.contents.pair_ptr = p;

  return e;
}

// We should write a list argument version of make_cons. 

/*
I think we want to treat the parsed input as a regular AST.

  p
 /\
2 /\
 5 /\
  8  null

Since arguments are already in a list, let's make use of them when
applying primitive procedures.

Note this is essentially call-by-reference since the argument list points
to an existing list in memory (We do this to allow for easy passing of
variable argument lengths to functions). But we musn't mutate the arg.

C doesn't allow variable-argument functions to be called with no arguments,
but I don't see a use case for it here, so I'm won't try adding support
for it.

We use Pair arg type because this is a purely internal-usage function
that we can guarantee is always a proper list.

Even if it's just one element like (list 1), the arg list will be (1) and
thus always has a car and cdr.
*/
Element make_list(const Pair *p)
{
  if (p->cdr.type_tag == PAIR && !p->cdr.contents.pair_ptr) {
    Element e = {
      .type_tag = PAIR,
      .contents.pair_ptr = NULL
    };

    // Any empty list is interchangeable with another.
    return make_cons(p->car, e);
  }

  return make_cons(p->car, make_list(p->cdr.contents.pair_ptr));
}

Element clone(const Element x)
{
  Element y = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  switch (x.type_tag) {
  // C's pass-by-value ensures numbers are always copied.
  case NUMBER:
  case BOOLEAN:
  // We copy the reference, which is what we want with non-atomic elements.
  case PAIR:
  // Primitives procedures are stored by the address of their functions.
  case PRIMITIVE_PROCEDURE:
  // Compounds procedures are same as Pairs.
  case COMPOUND_PROCEDURE:
    return x;
  // Since we store strings and symbol elements as pointers to manually
  // allocated memory, in order to truly copy them, we'll need to allocate
  // new memory.
  case SYMBOL:
    y.type_tag = x.type_tag;
    y.contents.symbol = string_alloc(strlen(x.contents.symbol));
    strcpy(y.contents.symbol, x.contents.symbol);
    return y;
  }
}

Element car(const Element x)
{
  // Add check?
  return x.contents.pair_ptr->car;
}

Element cdr(const Element x)
{
  return x.contents.pair_ptr->cdr;
}

Boolean is_true(const Element x)
{
  // Anything but a strict true is false.
  return x.type_tag == BOOLEAN && x.contents.truth;
}