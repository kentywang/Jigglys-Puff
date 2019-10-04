#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

static Boolean self_evaluating(Element);
static Boolean variable(Element);
static Boolean application(Element);

static Element lookup_variable_value(char *, Element);
static Element apply(Element, Element);

Element eval_dispatch(Element exp, Element env)
{
  if (self_evaluating(exp))
    return exp;
  if (variable(exp))
    return lookup_variable_value(exp.contents.symbol, env);
  // if (quoted(exp))
  //   return text_of_quotation(exp);
  // if (assignment(exp))
  //   return eval_assignment(exp, env);
  // if (definition(exp))
  //   return eval_definition(exp, env);
  // if (if_expression(exp))
  //   return eval_if(exp, env);
  // if (lambda(exp))
  //   return make_procedure(exp, env);
  // if (cond(exp))
  //   return eval_dispatch(cond_to_if(exp), env);
  // if (and(exp))
  //   return eval_and(exp, env);
  // if (or(exp))
  //   return eval_or(exp, env);
  // if (let(exp))
  //   return eval_dispatch(let_to_combination(exp), env);
  if (application(exp))
    return apply(exp, env);

  // TODO: Let print_element print to stderr. 
  fprintf(stderr, "Unknown expression type.\n");
  exit(BAD_EXPRESSION);
}

Boolean self_evaluating(Element exp)
{
  // TODO: Check if string too.
  return exp.type_tag == NUMBER;
}

Boolean variable(Element exp)
{
  return exp.type_tag == SYMBOL;
}


Boolean application(Element exp)
{
  return exp.type_tag == PAIR;
}

Element lookup_variable_value(char *var, Element env)
{
  Binding b = find_binding(var, env);

  if (b.variable)
    return b.value;

  fprintf(stderr, "Unbound variable: %s\n", var);
  exit(UNBOUND_VARIABLE);
}

Element apply(Element exp, Element env)
{

}