#include <stdio.h>
#include "lisp.h"

static void read_eval_print_loop(void);

Boolean verbosity;
Pair *input_ptr;

int main(int argc, const char *argv[])
{
  char c;

  while (--argc && **++argv == '-')
    while ((c = *++*argv))
      switch (c) {
      case 'v':
        verbosity = TRUE;
        break;
      default:
        fprintf(stderr, "Unexpected argument: -%c\n", c);
        return UNEXPECTED_ARG;
      }

  read_eval_print_loop();
}

void read_eval_print_loop(void)
{
  Element exp;
  Element val;
  Element env = setup_environment();

  save(env.contents.pair_ptr); // Global env should never be GCed.

  while (TRUE) {
    // Read
    read_input(&exp);

    // printf("READ:\n");
    // print_element(exp);

    // Eval
    val = eval_dispatch(exp, env);

    // Print
    print_element(val);

    // printf("ENV:\n");
    // print_element(env);

    // Free memory step?      
  }

  forget(); // Release global env from memory (not strictly needed here).
}
