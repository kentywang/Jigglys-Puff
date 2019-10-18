#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static Boolean self_evaluating(const Element);
static Boolean variable(const Element);
static Boolean application(const Element);
static Boolean special_form(char *, const Element);

static Element make_procedure(const Element, const Element);
static Element apply(const Element, const Element);
static Element list_of_values(Element, const Element);
static Element make_procedure(const Element, const Element);
static Element apply_compound(const Element, Pair *);
static Element eval_sequence(const Element, const Element);
static Element eval_definition(const Element, const Element);
static void define_variable(const Element, const Element, const Element);
static Element eval_if(const Element, const Element);

static Element procedure_parameters(const Element);
static Element procedure_body(const Element);
static Element procedure_environment(const Element);
static Element text_of_quotation(const Element);
static Element definition_variable(const Element);
static Element definition_value(const Element);

Element eval_dispatch(const Element exp, const Element env)
{
  // printf("EVAL:\n");
  // print_element(exp);

  if (self_evaluating(exp))
    return exp;
  if (variable(exp))
    return lookup_variable_value(exp.contents.symbol, env);
  if (special_form(QUOTE, exp))
    return text_of_quotation(exp);
  // if (assignment(exp))
  //   return eval_assignment(exp, env);
  if (special_form(DEFINE, exp))
    return eval_definition(exp, env);
  if (special_form(IF, exp))
    return eval_if(exp, env);
  if (special_form(LAMBDA, exp))
    return make_procedure(exp, env);
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

Boolean self_evaluating(const Element exp)
{
  // TODO: Check if string too.
  return exp.type_tag == NUMBER;
}

Boolean variable(const Element exp)
{
  return exp.type_tag == SYMBOL;
}

Boolean application(const Element exp)
{
  return exp.type_tag == PAIR;
}

Boolean special_form(char *symbol, const Element exp)
{
  return (
    exp.type_tag == PAIR &&
    car(exp).type_tag == SYMBOL &&
    strcmp(car(exp).contents.symbol, symbol) == 0
  );
}

Element apply(const Element exp, const Element env)
{
  Element procedure;
  Pair *arguments;

  // Since these single-use abstractions are just aliases, let's just
  // define them within this function.
  Element (*operator)(const Element) = &car;
  Element (*operands)(const Element) = &cdr;

  // If this was the top level application from input, then we release the
  // input AST from memory since we don't need it from this point forward.
  // Otherwise, we're releasing a dummy pointer.
  // We could move the release to after apply is done, so I wouldn't need
  // the dummies on the stack, but I want to verify that my logic is sound.
  if (input_ptr) {
    // Clear so that we don't release again on recursive eval calls.
    input_ptr = NULL;

    procedure = eval_dispatch((*operator)(exp), env);
    arguments = list_of_values((*operands)(exp), env).contents.pair_ptr;

    forget();
  } else {
    procedure = eval_dispatch((*operator)(exp), env);
    arguments = list_of_values((*operands)(exp), env).contents.pair_ptr;
  }

  if (procedure.type_tag == PRIMITIVE_PROCEDURE)
    // Does this allow null args?
    return (*procedure.contents.func_ptr)(arguments);
  else if (procedure.type_tag == COMPOUND_PROCEDURE)
    return apply_compound(procedure, arguments);

  // Not a procedure. TODO: Print operator.
  fprintf(stderr, "Not a procedure.\n");
  exit(NOT_PROCEDURE);
  return procedure;
}

Element list_of_values(Element operands, const Element env)
{
  Element empty = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  if (!operands.contents.pair_ptr) {
    return empty;
  }

  Pair *p = get_next_free_ptr();

  Element e = {
    .type_tag = PAIR,
    .contents.pair_ptr = p
  };

  save(p);

  p->car = clone(eval_dispatch(car(operands), env));

  while (cdr(operands).contents.pair_ptr) {
    p->cdr.contents.pair_ptr = get_next_free_ptr();
    p = p->cdr.contents.pair_ptr;
    operands = cdr(operands);
    p->car = clone(eval_dispatch(car(operands), env));
  }

  forget();

  p->cdr = empty;

  return e;
}

Element make_procedure(const Element exp, const Element env)
{
  Element e = {
    .type_tag = COMPOUND_PROCEDURE,
    .contents.pair_ptr = make_cons(
      car(cdr(exp)), // Lambda parameters
      make_cons(
        cdr(cdr(exp)), // Lambda body
        env
      )
    ).contents.pair_ptr
  };

  return e;
}

Element apply_compound(const Element procedure, Pair *arguments)
{
  Element e = {
    .type_tag = PAIR,
    .contents.pair_ptr = arguments
  };

  // Use e to hold frame.
  e = make_frame(procedure_parameters(procedure), e);

  save(e.contents.pair_ptr);

  // Use e to hold env.
  e = extend_environment(e, procedure_environment(procedure));

  // Update saved pointer for GC from preserving frame to preserving env.
  forget();
  save(e.contents.pair_ptr);

  // Use e to hold return value.
  e = eval_sequence(procedure_body(procedure), e);

  // Release any cons created in procedure application.
  while (cons_pointers--)
    forget();

  // Once we finish evaluating the compound procedure application, the env
  // is safe to discard, so we remove it from GC preservation.
  forget();

  return e;
}

// The compiler may perform tail-call optimization here. But we should
// rework evaluation into a giant while loop with a stack to explicitly
// control it.
Element eval_sequence(const Element exps, const Element env)
{
  // Are there more expressions after the head?
  if (cdr(exps).contents.pair_ptr) {
    eval_dispatch(car(exps), env);
    return eval_sequence(cdr(exps), env);
  }

  return eval_dispatch(car(exps), env);
}

Element eval_definition(const Element exp, const Element env)
{
  define_variable(
    definition_variable(exp),
    eval_dispatch(definition_value(exp), env),
    env
  );

  Element e = {
    .type_tag = PAIR,
    .contents.pair_ptr = NULL
  };

  // Return not important.
  return e;
}

/*
   f
  /\
 /\ \
x 5 /\
   /\ null
  y 6
*/
void define_variable(
  const Element var,
  const Element val,
  const Element env
)
{
  // TODO: Add check to see if variable exists in first frame.
  // If we had our own version of set-car!, we could use it here.
  env.contents.pair_ptr->car = make_cons(
    make_cons(var, val),
    first_frame(env)
  );
}

Element eval_if(const Element exp, const Element env)
{
  if (is_true(eval_dispatch(car(cdr(exp)), env))) {
    return eval_dispatch(car(cdr(cdr(exp))), env);
  }

  return eval_dispatch(car(cdr(cdr(cdr(exp)))), env);
}

Element text_of_quotation(const Element exp)
{
  return car(cdr(exp));
}

Element procedure_parameters(const Element exp)
{
  return car(exp);
}

Element procedure_body(const Element exp)
{
  return car(cdr(exp));
}

Element procedure_environment(const Element exp)
{
  return cdr(cdr(exp));
}

Element definition_variable(const Element exp)
{
  return car(cdr(exp));
}

Element definition_value(const Element exp)
{
  return car(cdr(cdr(exp)));
}