#include "sche_lib.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token create_symbol_token(const char *symbol) {
  Token token;
  token.type = TOKEN_TYPE_SYMBOL;
  token.value.s = strdup(symbol); // Allocate memory for the symbol
  return token;
}

Token create_number_token(int number) {
  Token token;
  token.type = TOKEN_TYPE_NUMBER;
  token.value.num = number;
  return token;
}

Token create_par_open_token() {
  Token token;
  token.type = TOKEN_TYPE_PAR_OPEN;
  return token;
}

Token create_par_close_token() {
  Token token;
  token.type = TOKEN_TYPE_PAR_CLOSE;
  return token;
}

void token_free(Token *token) {
  if (token->type == TOKEN_TYPE_SYMBOL) {
    free((char *)token->value.s); // Free dynamically allocated string
  }
}

void token_list_free(TokenList *token_list) {
  if (!token_list) {
    return;
  }
  if (token_list->tokens) {
    for (size_t i = 0; i < token_list->tokens_n; i++) {
      token_free(&token_list->tokens[i]);
    }
    token_list->tokens_n = 0;
    free(token_list->tokens);
    token_list->tokens = NULL;
  }
}

// Tokenize function
TokenList tokenize(const char *input) {
  if (input == NULL) {
    return (TokenList){.tokens_n = 0, .tokens = NULL};
  }

  char *s = strdup(input);
  if (!s) {
    perror("strdup");
    exit(EXIT_FAILURE);
  }

  const size_t max_tokens = strlen(s);
  TokenList result = {.tokens_n = 0,
                      .tokens = calloc(max_tokens, sizeof(Token))};

  if (!result.tokens) {
    perror("calloc");
    free(s);
    exit(EXIT_FAILURE);
  }

  const char *delimiters = " \t\n";
  char *token = strtok(s, delimiters);

  while (token != NULL) {
    if (strcmp(token, "(") == 0) {
      result.tokens[result.tokens_n++] = create_par_open_token();
    } else if (strcmp(token, ")") == 0) {
      result.tokens[result.tokens_n++] = create_par_close_token();
    } else {
      const char *start = token;
      while (*start != '\0') {
        if (*start == '(') {
          result.tokens[result.tokens_n++] = create_par_open_token();
          start++;
        } else if (*start == ')') {
          result.tokens[result.tokens_n++] = create_par_close_token();
          start++;
        } else {
          const char *end = start;

          // curring to delimiter + doing some checks
          bool has_alphas = false;
          bool has_digits = false;
          int dots_counter = 0;
          while (*end != '\0' && *end != '(' && *end != ')') {
            if (isalpha(*end)) {
              has_alphas = true;
            }
            if (isdigit(*end)) {
              has_digits = true;
            }
            if (*end == '.') {
              dots_counter++;
            }
            end++;
          }

          size_t length = end - start;
          char *symbol = strndup(start, length);
          if (has_digits && !has_alphas && dots_counter <= 1) {
            result.tokens[result.tokens_n++] =
                create_number_token(atoi(symbol));
          } else {
            result.tokens[result.tokens_n++] = create_symbol_token(symbol);
          }
          free(symbol);
          start = end;
        }
      }
    }

    token = strtok(NULL, delimiters);
  }

  free(s);
  void *resized_tokens =
      realloc(result.tokens, result.tokens_n * sizeof(Token));
  if (result.tokens_n > 0 && !resized_tokens) {
    perror("realloc");
    token_list_free(&result);
    exit(EXIT_FAILURE);
  }
  result.tokens = resized_tokens;

  return result;
}

void expression_free(Expression *expr) {
  if (expr == NULL) {
    return;
  }
  if (expr->type == EXPR_TYPE_SUBEXPR) {
    expression_free(expr->subexpr);
  }
  if (expr->type == EXPR_TYPE_NAME) {
    free((char *)expr->value.s);
  }
  if (expr->next != NULL) {
    expression_free(expr->next);
  }
  free(expr);
}

Expression *parse(TokenList token_list) {
  const size_t tokens_n = token_list.tokens_n;
  if (tokens_n == 0) {
    return NULL;
  }
  const Token *tokens = token_list.tokens;
  if (tokens_n != 0 && (tokens[0].type != TOKEN_TYPE_PAR_OPEN ||
                        tokens[tokens_n - 1].type != TOKEN_TYPE_PAR_CLOSE)) {
    perror("Unbalanced parenthesis");
    return NULL;
  }
  if (tokens_n < 3) {
    perror("Not enough tokens");
    return NULL;
  }
  Expression *const first_expr = malloc(sizeof(Expression));
  first_expr->next = NULL;
  first_expr->value.s = NULL;
  Expression *current_expr = first_expr;
  const size_t idx_of_closing_par = tokens_n - 1;
  for (size_t i = 1; i < idx_of_closing_par; ++i) {
    switch (tokens[i].type) {
    case TOKEN_TYPE_NUMBER:
      current_expr->type = EXPR_TYPE_INT;
      current_expr->value.num = tokens[i].value.num;
      break;
    case TOKEN_TYPE_SYMBOL:
      current_expr->type = EXPR_TYPE_NAME;
      current_expr->value.s = strdup(tokens[i].value.s);
      break;
    case TOKEN_TYPE_PAR_OPEN: {
      current_expr->type = EXPR_TYPE_SUBEXPR;
      size_t subexpr_tokens_n = 1;
      {
        size_t opening_parenthesis_n = 1;
        while (opening_parenthesis_n != 0) {
          if (tokens[i + subexpr_tokens_n].type == TOKEN_TYPE_PAR_OPEN) {
            ++opening_parenthesis_n;
          } else if (tokens[i + subexpr_tokens_n].type ==
                     TOKEN_TYPE_PAR_CLOSE) {
            --opening_parenthesis_n;
          }
          ++subexpr_tokens_n;
        }
      }
      current_expr->subexpr = parse(
          (TokenList){.tokens_n = subexpr_tokens_n, .tokens = tokens + i});
      i += subexpr_tokens_n - 1;
      break;
    }
    case TOKEN_TYPE_PAR_CLOSE:
      assert(false);
    }

    const bool we_have_some_tokens_left = i != idx_of_closing_par - 1;
    if (we_have_some_tokens_left) {
      current_expr->next = malloc(sizeof(Expression));
      current_expr = current_expr->next;
      current_expr->next = NULL;
    }
  }
  return first_expr;
}

static inline const Expression *exp_get_nth(const Expression *expr, size_t n) {
  assert(expr != NULL);
  while (0 < n--) {
    expr = expr->next;
  }
  return expr;
}

static size_t exp_list_len(const Expression *expr) {
  size_t len = 0;
  while (expr != NULL) {
    ++len;
    expr = expr->next;
  }
  return len;
}

#define MAKE_BINARY_OP(op_name, op_sign)                                       \
  static long op_name(int a, int b) { return a op_sign b; }

MAKE_BINARY_OP(f_add, +)
MAKE_BINARY_OP(f_sub, -)
MAKE_BINARY_OP(f_mul, *)
MAKE_BINARY_OP(f_div, /)
MAKE_BINARY_OP(f_rem, %)

static const Function functions[] = {
    {.name = "+", .run = f_add},
    {.name = "-", .run = f_sub},
    {.name = "*", .run = f_mul},
    {.name = "/", .run = f_div},
    {.name = "%", .run = f_rem},
};

static int exp_get_nth_numeric_value(const Expression *expr, size_t n) {
  const Expression *nested_expr = exp_get_nth(expr, n);
  if (nested_expr->type == EXPR_TYPE_INT) {
    return nested_expr->value.num;
  } else if (nested_expr->type == EXPR_TYPE_SUBEXPR) {
    return eval_expr_list(nested_expr->subexpr);
  }
  perror("Invalid argument type");
  exit(4);
}

int eval_expr_list(const Expression *expr) {
  {
    bool expression_begins_with_name = expr->type == EXPR_TYPE_NAME;
    if (!expression_begins_with_name) {
      perror("Expression must begin with a name");
      exit(1);
    }
  }

  const char *name = expr->value.s;
  const Function *f = NULL;
  for (size_t i = 0; i < sizeof(functions) / sizeof(functions[0]); ++i) {
    if (strcmp(name, functions[i].name) == 0) {
      f = &functions[i];
      break;
    }
  }

  if (f == NULL) {
    perror("Unknown function");
    exit(2);
  }

  const int args_n = exp_list_len(expr) - 1;
  if (args_n < 2) {
    fprintf(stderr, "Wrong number of arguments for function %s\n", name);
    exit(3);
  }

  // foldl'
  // 0 is the funciton, start from 1
  int acc = exp_get_nth_numeric_value(expr, 1);
  for (int i = 2; i <= args_n; ++i) {
    int new_arg = exp_get_nth_numeric_value(expr, i);
    acc = f->run(acc, new_arg);
  }
  return acc;
}

int eval(const char *s) {
  TokenList tokens = tokenize(s);

  Expression *expr = parse(tokens);
  token_list_free(&tokens);

  int result = eval_expr_list(expr);
  expression_free(expr);

  return result;
}
