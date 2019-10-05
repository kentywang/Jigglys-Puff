#include <stdio.h>
#include "lisp.h"

/*
My approach will be to garbage collect on a fixed region of memory for
list-structured data, but strings/symbols will actually be stored in
dynamically-allocated memory via malloc/free. Though we could store them
in lists eventually.

Should we want to explicitly control call stacks, we can either store the
stack within or as a separate structure from the GCed heap. Since stacks
never build up if tail recursion is implemented properly, it doesn't make sense to keep it with other data that does need to be garbage collected.

I think we should use the agnostic Element type more than the Pair pointer.

Questions:
- How do you differentiate between newlines (ignored) and the enter key?
  A: Don't think you can, you need to listen to keyboard's shift.
- Do we need to manually set CDR to null because of GC?
- Should I create an abstraction layer to convert an argument list into
  a va_list for primitive functions to apply?
- Will staticall-declared functions be accessible from the outside?

Tests:
  (1(  2  3  )4)
   (1 2 3)
    (1 2 (3 4 (5)) 6 7)
    (1(  -2  3a  )4)
apple
   apple

Todos:
- Devise method of unit testing.
- Remove unneeded wrapping of Pair pointers with an Element, since they're
  initialized with PAIR type tags already. (GC might complicate this, we'll
  need to reset the type_tag and content in get_next_free_ptr! Hmm, but that
  approach might mean we'd need to manually set the CDR to the empty pair
  instead on relying on uninitialized Elements to always be empty pairs, or
  would it?)
- Can we convert the functions that take Element pointers into functions that
  take Elements? (read)
- Make backend more C-like.
- Make read robust against newlines, live stdin.
- Protect against overly long input words.
- Unify procedures used in both interpreted and backend, or separate them.
- Expand verbose to eval/apply.
- Can we change a lot of our Element functions to Pair pointers without
  breaking their behavior within interpretation?
- Implement manual tail recursion.
- GC!

Lessons learned:
- For mutating an object's pointer member, I can't pass the pointer into
  a function; I must pass the object.
- fseek and fgets don't work with live stdin (as opposed to from a text file).
  For fgets, the program waits until we have _new_ input before getting.
- () and [] has higher precedence than ++ and *, despite some online sources
  stating otherwise.

*/

static void read_eval_print_loop(const Boolean);

int main(int argc, const char *argv[])
{
  char c;
  Boolean verbose = FALSE;

  while (--argc && **++argv == '-')
    while ((c = *++*argv))
      switch (c) {
      case 'v':
        verbose = TRUE;
        break;
      default:
        fprintf(stderr, "Unexpected argument: -%c\n", c);
        return UNEXPECTED_ARG;
      }

  read_eval_print_loop(verbose);
}

void read_eval_print_loop(const Boolean verbose)
{
  Element exp;
  Element val;
  Element env = setup_environment();

  while (TRUE) {
    // Read
    read_input(&exp, verbose);

    // Eval
    val = eval_dispatch(exp, env);

    // Print
    print_element(val);
    printf("\n");

    // Free memory step?
  }
}
