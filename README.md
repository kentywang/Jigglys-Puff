## (jigg lisp puff)

A Scheme interpreter in C developed for exercise 5.51 of SICP.

My approach will be to garbage collect on a fixed region of memory for
list-structured data, but strings/symbols will actually be stored in
dynamically-allocated memory via malloc/free. Though we could store them
in lists eventually.

Should we want to explicitly control call stacks, we can either store the
stack within or as a separate structure from the GCed heap. Since stacks
never build up if tail recursion is implemented properly, it doesn't make sense to keep it with other data that does need to be garbage collected.

I think we should use the agnostic Element type more than the Pair pointer.

### Questions
- How do you differentiate between newlines (ignored) and the enter key?
  A: Don't think you can, you need to listen to keyboard's shift.
- Do we need to manually set CDR to null because of GC?
- Should I create an abstraction layer to convert an argument list into
  a va_list for primitive functions to apply?
- Will staticall-declared functions be accessible from the outside?

### Tests
  (1(  2  3  )4)
   (1 2 3)
    (1 2 (3 4 (5)) 6 7)
    (1(  -2  3a  )4)
apple
   apple
(define fact
  (lambda (n)
    (if (= n 1)
        1
        (* (fact (+ n -1)) n))))
(define cons
  (lambda (x y)
    (lambda (m) (m x y))))
(define car
  (lambda (z)
    (z (lambda (p q) p))))

### Todos
- Devise method of unit testing.
- Remove unneeded wrapping of Pair pointers with an Element, since they're
  initialized with PAIR type tags already. (GC might complicate this, we'll
  need to reset the type_tag and content in get_next_free_ptr! Hmm, but that
  approach might mean we'd need to manually set the CDR to the empty pair
  instead on relying on uninitialized Elements to always be empty pairs, or
  would it?)
- Can we convert the functions that take Element pointers into functions that
  take Elements? (read?) Maybe make it Pair pointers so it might be left-right
  expression agnostic?
- Make backend more C-like.
- Make read robust against newlines, live stdin.
- Protect against overly long input words.
- Unify procedures used in both interpreted and backend, or separate them.
- Expand verbose to eval/apply.
- Can we change a lot of our Element functions to Pair pointers without
  breaking their behavior within interpretation?
- Implement manual tail recursion.
- GC!
- Writeup architecture readme.
- I waiver between relying on default initialization for Elements and
  explicitly setting the values. Choose one. (Also, we might not be able to
  rely on uninitialized values being what we expect on automatic variables.)
- Am I not storing pairs for strings? Makes sense, right, because how would a
  string be kept in list-structured memory?
- I'm not treating single-quotes in quotes literally; it's still converting
  them to a quote list. Not sure if this affects any behavior.
- Add full list of primitives.
- Does empty list work? (i.e. '())
- Support reading boolean.
- Use Pair pointers for places where we need roots?
- Add logging for stack pointer.
- Store symbols/strings only once.
- Add option to trigger GC on every REPL cycle.
- Does my approach GC the input AST if we have a non-procedure application
  pair?
- How can we preserve local variable (both from arguments and manually-
  defined) references across GCs?

### Garbage Collection
There's 5 classes of data on the heap with varying lifespans:
- global environment (persistent) [x]
- input AST (from creation on read until after evaluating all its
  subexpressions) [x]
- evaluated arguments (from creation within eval/apply until after env is
  extended by a frame with them) [x]
- extended environment (from creation within eval/apply until after apply is
  finished and we return the value to the calling function) [x]
- pairs created from primitive procedure calls (e.g. cons, list) (from
  creation anywhere within eval/apply to return of calling function，since
  if it's passed to another function then its reference is preserved in its
  extendend environment) []
- the return value, if it's a Pair pointer.

If we can guarantee that these will be preserved across swaps between free
and working memory, we should be fine. Any pointer variable that refers to
one of the pairs in memory will be redirected with a broken heart.

Currently I've implemented a simplified GC for objects created during
procedure application, where all objects are persisted in memory until the
application is over. This can be optimized to only persist what is needed
(what's referenced by variables). I don't think we need to persist the return
value since that'll be the evaluated argument of any subsequent application.

Wait... couldn't we just add all defined variables to the stack and remove
them upon scope ending, instead of preserving all conses within an
application? Not sure how this would work with lexical scoping though.

### Overview of features added:
- Verbose option for tracing evaluation (WIP)
- Tail-optimized recursion (WIP)
- Lambda expressions
- Quotations
- Converts single quotes in input into quote lists and does the opposite
  conversion on output.
- Definitions
- Conditional expression and booleans.

### Lessons learned
- For mutating an object's pointer member, I can't pass the pointer into
  a function; I must pass the object.
- fseek and fgets don't work with live stdin (as opposed to from a text file).
  For fgets, the program waits until we have _new_ input before getting.
- () and [] has higher precedence than ++ and *, despite some online sources
  stating otherwise.