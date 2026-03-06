#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum LEXEME {
  NUM,
  ADD,
  SUB,
  MUL,
  DIV,
};

typedef struct token {
  enum LEXEME type;
  char value;
} token_t;

typedef struct token_builder {
  token_t token;
  char *str;
} token_builder_t;

typedef struct token_node {
  struct token_node *left;
  token_t token;
  struct token_node *right;
} token_node_t;

typedef struct token_node_builder {
  token_node_t *node;
  char *str;
} token_node_builder_t;

void print_file(FILE *fp) {
  char buff[100];
  while (fgets(buff, sizeof(buff), fp) != NULL) {
    printf("%s", buff);
  }
  pclose(fp);
}

int is_num(char *str) {
  char c;
  int i = 0;
  while ((c = str[i]) != '\0') {
    if (!isdigit(str[i])) {
      return 0;
    }
    i++;
  }
  return 1;
}

token_node_t *new_node(token_node_t *left, token_t token, token_node_t *right) {
  token_node_t *new = malloc(sizeof(token_node_t));
  new->left = NULL;
  new->right = NULL;
  new->token = token;
  return new;
}

/*void parse(token_node_t *root) {
  if (root != NULL) {
    analyze(root->left);
    analyze(root->right);
    if (root->token.token_type == NUMBER) {
      printf("push %d\n", root->token.value);
    } else {
      printf("pop rax\npop rbx\nadd rax rbx\npush rax\n");
    }
  }
}*/

token_builder_t analyze(char *str) {
  int len = strpbrk(str, "1234567890") - str;

  if (len > 0) {
    char num_str[len];
    memcpy(&num_str, str, len);
    return (token_builder_t){(token_t){NUM, atoi(num_str)}, str + len};
  }

  enum LEXEME lex;

  switch (str[0]) {
  case ' ':
    return analyze(str + 1);
    break;
  case '+':
    lex = ADD;
    break;
  case '-':
    lex = SUB;
    break;
  case '*':
    lex = MUL;
    break;
  case '/':
    lex = DIV;
    break;
  default:
    abort();
    break;
  }

  return (token_builder_t){(token_t){lex, str[0]}, str + 1};
}

token_node_builder_t factor(char *str) {
  token_builder_t analyzer = analyze(str);

  if (analyzer.token.type == NUM) {
    return (token_node_builder_t){new_node(NULL, analyzer.token, NULL),
                                  analyzer.str};
  }

  abort();
}

token_node_builder_t term(char *str) {
  token_node_builder_t left = factor(str);
  str = left.str;
  token_builder_t analyzer = analyze(str);
  str = analyzer.str;
  token_t token = analyzer.token;

  while (analyzer.token.type == MUL || analyzer.token.type == DIV) {
    token_node_builder_t right = factor(str);
    str = right.str;
    return (token_node_builder_t){
        new_node(left.node, token, right.node),
        str,
    };
  }
}

int main(int argc, char *argv[]) {
  // token_node_t *root = new_node((token_t) {OPERATOR, '+'});
  // root->left = new_node((token_t) {NUMBER, 1});
  // root->right = new_node((token_t) {NUMBER, 2});

  // analyze(root);

  char *filename = argc == 2 ? argv[1] : "main.cell";

  puts("==  reading src  ==");
  FILE *src = fopen(filename, "r");
  FILE *out = fopen("cell.asm", "w");

  // == analysis ==
  // -- lexical analysis --
  /*char line[100];
  token_t tokens[100];
  char *token;
  int tokens_len = 0;
  while (fgets(line, sizeof(line), src)) {
    line[strcspn(line, "\n")] = '\0';
    token = strtok(line, " ");
    while (token != NULL) {
      if (is_num(token)) {
        tokens[tokens_len] = (token_t){NUMBER, atoi(token)};
      } else {
        tokens[tokens_len] = (token_t){OPERATOR, token[0]};
      }
      tokens_len++;
      token = strtok(NULL, " ");
    }
  }

  for (int j = 0; j < tokens_len; j++) {
    if (tokens[j].token_type == NUMBER) {
      printf("<id, %d> ", tokens[j].value);
    } else {
      printf("<%c> ", tokens[j].value);
    }
  }*/

  // STAGES OF PROCESSING
  // lecical analyzer gets next token
  // syntax analyzer analyzes all additions
  // syntax analyzer analyzes all multipications

  // -- syntax analysis --
  // multiplication

  // addition

  fclose(src);
  printf("\n\n");

  // == synthesis ==
  puts("==  writing assembly  ==");
  fprintf(out, ".intel_syntax noprefix\n"
               ".section .text\n"
               ".global _start\n"
               "\n"
               "_start:\n");
  for (int i = 0; i < tokens_len; i++) {
    if (tokens[i].type == NUMBER) {
      fprintf(out, "  push %d\n", tokens[i].value);
    } else if (tokens[i].type == OPERATOR) {
      fprintf(out, "  pop rbx\n"
                   "  pop rax\n"
                   "  add rax, rbx\n"
                   "  push rax\n");
    }
  }
  fprintf(out, "  mov rax, 60\n"
               "  pop rdi\n"
               "  syscall\n");
  fclose(out);
  print_file(fopen("cell.asm", "r"));

  return 0;
}
