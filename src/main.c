#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 4
#define STACK_GROW 2

// lexical analyzer tokens (lexemes)
typedef enum {
  TOK_EOF,
  TOK_ATOM,
  TOK_OP,
} tag_t;

// string stack
typedef struct {
  char *str;
  size_t index;
} str_feeder_t;

// lexical analyzer token
typedef struct {
  tag_t tag;
  union {
    char op;
    int atom;
  };
} token_t;

// lexical analyzer stack
typedef struct {
  token_t *data;
  size_t top;
  size_t size;
} lex_stack_t;

// token node for trees
typedef struct token_node {
  struct token_node *left;
  token_t data;
  struct token_node *right;
} token_node_t;

typedef struct {
  float left;
  float right;
} binding_t;

void str_rev(char *s) {
  int l = 0;
  int r = strlen(s) - 1;
  while (l < r) {
    char t = s[l];
    s[l] = s[r];
    s[r] = t;
    l++;
    r--;
  }
}

bool char_is_num(char c) { return c >= '0' && c <= '9'; }

str_feeder_t *str_feeder_new(char *str) {
  // TODO: error checking
  int strc = strlen(str);
  str_feeder_t *feeder = malloc(sizeof(str_feeder_t));

  feeder->str = malloc(sizeof(char) * strc);
  memcpy(feeder->str, str, strc);
  feeder->index = 0;

  return feeder;
}

char str_feeder_next(str_feeder_t *feeder) {
  char chr = feeder->str[feeder->index];
  feeder->index++;
  return chr;
}

char str_feeder_peek(str_feeder_t *feeder) {
  return feeder->str[feeder->index];
}

void str_feeder_free(str_feeder_t *feeder) {
  free(feeder->str);
  free(feeder);
}

lex_stack_t *lex_stack_new() {
  lex_stack_t *stack = malloc(sizeof(lex_stack_t));

  stack->data = malloc(sizeof(token_t) * STACK_SIZE);
  stack->top = -1;
  stack->size = STACK_SIZE;

  return stack;
}

void lex_stack_push(lex_stack_t *stack, token_t tok) {
  if (stack->top + 1 >= stack->size) {
    token_t *tmp =
        realloc(stack->data, sizeof(token_t) * stack->size * STACK_GROW);
    stack->data = tmp;
    stack->size *= STACK_GROW;
  }
  stack->top++;
  stack->data[stack->top] = tok;
}

token_t lex_stack_peek(lex_stack_t *stack) { return stack->data[stack->top]; }

token_t lex_stack_pop(lex_stack_t *stack) {
  stack->top--;
  return stack->data[stack->top];
}

lex_stack_t *lex_stack_rev(lex_stack_t *lex) {
  lex_stack_t *new = lex_stack_new();
  for (int i = lex->top; i >= 0; i--) {
    lex_stack_push(new, lex_stack_pop(lex));
  }
  return new;
}

void lex_stack_print(lex_stack_t *lex) {
  for (int i = lex->top; i >= 0; i--) {
    token_t tok = lex->data[i];
    if (tok.tag == TOK_ATOM) {
      printf("<tag, %d> ", tok.atom);
    } else if (tok.tag == TOK_OP) {
      printf("<%c> ", tok.op);
    } else if (tok.tag == TOK_EOF) {
      printf("<EOF> ");
    }
  }
}

void lex_stack_free(lex_stack_t *stack) {
  free(stack->data);
  free(stack);
}

token_node_t *token_node_new() {
  token_node_t *node = malloc(sizeof(token_node_t));
  node->left = NULL;
  node->right = NULL;
  // node->data = (token_t){NULL, .atom = NULL};
  return node;
}

token_node_t *token_node_new_data(token_t data) {
  token_node_t *node = token_node_new();
  node->data = data;
  return node;
}

void token_node_print(token_node_t *node) {
  if (node != NULL) {
    if (node->data.tag == TOK_OP) {
      printf("(");
      printf("%c ", node->data.op);
      token_node_print(node->left);
      printf(" ");
      token_node_print(node->right);
      printf(")");
    } else {
      printf("%d", node->data.atom);
    }
  }
}

void token_node_free(token_node_t *node) {
  if (node != NULL) {
    token_node_free(node->left);
    token_node_free(node->right);
    free(node);
  }
}

lex_stack_t *lex_analyze(char *s) {
  str_feeder_t *strf = str_feeder_new(s);
  lex_stack_t *lexs = lex_stack_new();
  size_t len = strlen(strf->str) - 1;

  while (strf->index <= len + 1) {
    char chr = str_feeder_next(strf);
    token_t tok;

    if (char_is_num(chr)) {
      int num = chr - '0';
      while (char_is_num(str_feeder_peek(strf))) {
        num = num * 10 + str_feeder_next(strf);
      }
      tok = (token_t){TOK_ATOM, .atom = num};
    } else {
      switch (chr) {
      case '\n':
        tok = (token_t){TOK_EOF};
        break;
      case '+':
      case '-':
      case '*':
      case '/':
        tok = (token_t){TOK_OP, .op = chr};
        break;
      }
    }

    lex_stack_push(lexs, tok);
  }

  str_feeder_free(strf);
  return lexs;
}

binding_t binding_power(token_t op) {
  switch (op.op) {
  case '+':
  case '-':
    return (binding_t){1.0, 1.1};
    break;
  case '*':
  case '/':
    return (binding_t){2.0, 2.1};
    break;
  default:
    return (binding_t){-1, -1};
  }
}

token_node_t *parse_lex(lex_stack_t *lex, float min_bp) {
  token_t left = lex_stack_pop(lex);
  token_node_t *node = token_node_new();

  while (true) {
    token_t op = lex_stack_pop(lex);
    if (op.tag == TOK_EOF) {
      break;
    }
    binding_t power = binding_power(op);
    if (power.left < min_bp) {
      break;
    }
    token_node_t *right = parse_lex(lex, power.right);
    node->data = op;
    node->left = token_node_new_data(left);
    node->right = right;
  }
  return node;
}

int main(int argc, char *argv[]) {
  // rough compillation process:
  // src -> str_feeder
  // str_feeder -> lex_stack
  // lex_stack -> token_nodes
  // token_nodes -> assembly

  char *filename = argc == 2 ? argv[1] : "main.cell";

  // == analysis ==
  puts("==  reading src  ==");
  FILE *src = fopen(filename, "r");

  char buff[256];
  fgets(buff, sizeof(buff), src);
  fclose(src);

  puts("-- lex stack --");
  lex_stack_t *lex_stack = lex_analyze(buff);
  lex_stack = lex_stack_rev(lex_stack);
  lex_stack_print(lex_stack);
  puts("");

  puts("-- syntax tree --");
  token_node_t *token_tree = parse_lex(lex_stack, 0.0);
  token_node_print(token_tree);
  puts("");

  lex_stack_free(lex_stack);
  token_node_free(token_tree);

  // == synthesis ==
  puts("==  writing asm  ==");
  FILE *out = fopen("cell.asm", "w");

  fclose(out);

  return 0;
}
