/* Global */
#define LAMBDA "lambda"
#define QUOTE "quote"
#define QUOTE_LENGTH 5 // The length of the word "quote"
#define DEFINE "define"
#define IF "if"
#define TRUE_SYMBOL "true"
#define FALSE_SYMBOL "false"
#define print_verbose verbosity && printf

typedef enum boolean Boolean;
typedef enum type Type;
typedef union value Value;
typedef struct element Element;
typedef struct pair Pair;
typedef struct binding Binding;

enum error_codes {
  BAD_IDENTIFIER,
  BAD_EXPRESSION,
  UNBOUND_VARIABLE,
  UNEXPECTED_ARG,
  NOT_PROCEDURE,
  ARITY_MISMATCH,
};

// Be explicit that false should be 0.
enum boolean {
  FALSE = 0,
  TRUE = 1
};

// Members of enum and union must be in same corresponding order for initial
// memory values to match their types.
// Be careful, though, since "initial values" means nothing after garbage
// collection.
struct element {
  enum {
    PAIR,
    NUMBER,
    SYMBOL,
    BOOLEAN,
    // The following are never read from input, and are never exposed to
    // users:
    PRIMITIVE_PROCEDURE,
    COMPOUND_PROCEDURE
  } type_tag;
  union {
    Pair *pair_ptr;
    int number;
    char *symbol;
    Boolean truth;
    // PRIMITIVE_PROCEDURE uses a Pair pointer like PAIR.
    Element (*func_ptr)(const Pair *);
    // Need to store string too.
  } contents;
};

struct pair {
  Element car;
  Element cdr;
};

// Just a single-use object to hold variables to load into memory in
// proper list-structure.
struct binding {
  char *variable;
  Element value;
};

/* main.c */
extern Boolean verbosity;
extern Pair *input_ptr;

/* read.c */
extern void read_input(Element *);

/* eval.c */
extern Element eval_dispatch(const Element, const Element);

/* print.c */
extern void print_element(const Element);

/* stack.c */
extern void save(Pair *);
extern void forget(void);

/* heap.c */
extern Pair *get_next_free_ptr(void);
extern char *string_alloc(int);

/* data.c */
extern Element make_list(const Pair *);
extern Element make_cons(const Element, const Element);
extern Element car(const Element);
extern Element cdr(const Element);
extern Element clone(const Element);
extern Boolean is_true(const Element);

/* env.c */
extern Element extend_environment(const Element, const Element);
extern Element setup_environment(void);
extern Binding find_binding(char *, Element);
extern Element lookup_variable_value(char *, Element);
extern Element make_frame(const Element, const Element);
extern Element first_frame(const Element);
extern Element enclosing_environment(const Element);
extern Boolean is_empty_environment(const Element);

/* primitive.c */
extern Binding initial_frame[];
extern Element load_frame(const Binding *);

/* util.c */
extern Boolean is_integer(char *);