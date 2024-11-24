#include "sche_lib.h"
#include "memory_tracker.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MemoryTracker *mt = NULL;
static void *memory_tracker_allocate(size_t size) {
  if (mt == NULL) {
    mt = memory_tracker_init(4096);
  }
  return memory_tracker_push(mt, size);
}
static void my_release_all() { memory_tracker_release(mt); }
#define ALLOCATOR memory_tracker_allocate
#define DEALLOCATOR my_release_all

TokenizerResult tokenize_with_allocator(const char *s, allocator alloc) {
  TokenizerResult result = {
      .status = TOKENIZER_SUCCESS,
      .token_list = {.tokens_n = 0, .tokens = NULL}
  };

  char *token, *string, *tofree;
  tofree = string = strdup(s);
  assert(tofree != NULL);

  const size_t max_tokens_n = strlen(s);
  Token *tmp_tokens = calloc(max_tokens_n, sizeof(Token));
  while ((token = strsep(&string, " \t\r\n")) != NULL) { // code by words
    while (*token != '\0') {                             // word by characters
      Token *new_token = tmp_tokens + result.token_list.tokens_n;
      ++result.token_list.tokens_n;
      if (*token == '(' || *token == ')') {
        new_token->type =
            (*token == '(') ? TOKEN_TYPE_PAR_OPEN : TOKEN_TYPE_PAR_CLOSE;
        ++token;
        continue;
      }

      char *end_of_number = NULL;
      long number = strtol(token, &end_of_number, 0);

      if (end_of_number != NULL && *end_of_number == '\0') { // digits only
        new_token->type = TOKEN_TYPE_NUMBER;
        new_token->value.num = number;
        break;
      } else if (end_of_number == token) { // no digits, non-digits + digits
        new_token->type = TOKEN_TYPE_NAME;
        size_t token_len = strlen(token);
        char *copy_of_token = alloc(token_len + 1);
        assert(copy_of_token != NULL);
        strncpy(copy_of_token, token, token_len);
        copy_of_token[token_len] = '\0';
        new_token->value.s = copy_of_token;
        break;
      } else if (end_of_number != NULL &&
                 *end_of_number != '\0') { // digits +
                                           // non-digits
        if (end_of_number == strpbrk(token, "()")) {
          new_token->type = TOKEN_TYPE_NUMBER;
          new_token->value.num = number;
          token = end_of_number;
        } else {
          result.status = TOKENIZER_ERROR_INVALID_NAME;
          goto cleanup;
        }
      } else {
        assert(false);
      }
    }
  }
  result.token_list.tokens = alloc(sizeof(Token) * result.token_list.tokens_n);
  memcpy(result.token_list.tokens,
         tmp_tokens,
         sizeof(Token) * result.token_list.tokens_n);
cleanup:
  free(tmp_tokens);
  free(tofree);
  return result;
}

TokenizerResult tokenize(const char *s) {
  return tokenize_with_allocator(s, ALLOCATOR);
}

ParserResult parse_with_allocator(size_t tokens_n, const Token tokens[],
                                  allocator alloc) {
  if (tokens_n > 0 && (tokens[0].type != TOKEN_TYPE_PAR_OPEN ||
                       tokens[tokens_n - 1].type != TOKEN_TYPE_PAR_CLOSE)) {
    return (ParserResult){.type = PARSE_ERROR_UNBALANCED_PAR, .expr = NULL};
  }
  if (tokens_n < 3) {
    return (ParserResult){.type = PARSE_ERROR_TOO_SHORT_EXPR, .expr = NULL};
  }
  Expression *const first_expr = alloc(sizeof(Expression));
  Expression *current_expr = first_expr;
  const size_t idx_of_closing_par = tokens_n - 1;
  for (size_t i = 1; i < idx_of_closing_par; ++i) {
    switch (tokens[i].type) {
    case TOKEN_TYPE_NUMBER:
      current_expr->type = EXPR_TYPE_INT;
      current_expr->value.num = tokens[i].value.num;
      break;
    case TOKEN_TYPE_NAME:
      current_expr->type = EXPR_TYPE_SUBEXPR;
      current_expr->value.s = tokens[i].value.s;
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
      ParserResult subexpr_parse_result =
          parse_with_allocator(subexpr_tokens_n, tokens + i, alloc);
      if (subexpr_parse_result.type != PARSE_SUCCESS) {
        return (ParserResult){.type = subexpr_parse_result.type, .expr = NULL};
      }
      current_expr->subexpr = subexpr_parse_result.expr;
      i += subexpr_tokens_n - 1;
      break;
    }
    case TOKEN_TYPE_PAR_CLOSE:
      assert(false);
    }

    const bool we_have_some_tokens_left = i != idx_of_closing_par - 1;
    if (we_have_some_tokens_left) {
      current_expr->next = alloc(sizeof(Expression));
      current_expr = current_expr->next;
      current_expr->next = NULL;
    }
  }
  return (ParserResult){.type = PARSE_SUCCESS, .expr = first_expr};
}

ParserResult parse(size_t tokens_n, const Token tokens[]) {
  return parse_with_allocator(tokens_n, tokens, ALLOCATOR);
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

static long f_add(size_t args_n, const long *args) {
  long sum = 0;
  for (size_t i = 0; i < args_n; ++i) {
    sum += args[i];
  }
  return sum;
}

static long f_sub(size_t args_n, const long *args) {
  (void)(args_n);
  return args[0] - args[1];
}

static long f_mul(size_t args_n, const long *args) {
  long product = 1;
  for (size_t i = 0; i < args_n; ++i) {
    product *= args[i];
  }
  return product;
}

static long f_div(size_t args_n, const long *args) {
  (void)(args_n);
  if (args[1] == 0) {
    fprintf(stderr, "Division by zero\n");
    assert(false);
  }
  return args[0] / args[1];
}

static long f_rem(size_t args_n, const long *args) {
  (void)(args_n);
  if (args[1] == 0) {
    fprintf(stderr, "Division by zero\n");
    assert(false);
  }
  return args[0] % args[1];
}

static const Function functions[] = {
    {.name = "+", .min_args_n = 2, .max_args_n = -1, .run = f_add},
    {.name = "-", .min_args_n = 2, .max_args_n = 2,  .run = f_sub},
    {.name = "*", .min_args_n = 2, .max_args_n = -1, .run = f_mul},
    {.name = "/", .min_args_n = 2, .max_args_n = 2,  .run = f_div},
    {.name = "%", .min_args_n = 2, .max_args_n = 2,  .run = f_rem},
};

EvalResult eval_expr_list(const Expression *expr) {
  {
    bool expression_begins_with_name = expr->type == EXPR_TYPE_SUBEXPR;
    if (!expression_begins_with_name) {
      return (EvalResult){.type = EVAL_ERROR_NAME_EXPECTED};
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
    return (EvalResult){.type = EVAL_ERROR_UNDEFINED_FUNCTION};
  }

  const int args_n = exp_list_len(expr) - 1;
  if (args_n < 0 || args_n < f->min_args_n ||
      (f->max_args_n != -1 && args_n > f->max_args_n)) {
    fprintf(stderr, "Wrong number of arguments for function %s\n", name);
    return (EvalResult){.type = EVAL_ERROR_WRONG_ARGS_N};
  }

  long *args = malloc(sizeof(long) * args_n);
  if (args == NULL) {
    return (EvalResult){.type = EVAL_ERROR_MEMORY_ALLOC};
  }

  EvalResult result = {.type = EVAL_SUCCESS};
  for (int i = 0; i < args_n; ++i) {
    const Expression *arg = exp_get_nth(expr, i + 1);
    if (arg->type == EXPR_TYPE_INT) {
      args[i] = arg->value.num;
    } else if (arg->type == EXPR_TYPE_SUBEXPR) {
      EvalResult subexpr_eval_result = eval_expr_list(arg->subexpr);
      if (subexpr_eval_result.type != EVAL_SUCCESS) {
        result = subexpr_eval_result;
        goto cleanup;
      }
      args[i] = subexpr_eval_result.value.num;
    } else {
      result = (EvalResult){.type = EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED};
      goto cleanup;
    }
  }
  result.value.num = f->run(args_n, args);
cleanup:
  free(args);
  return result;
}

EvalResult eval(const char *s) {
  TokenizerResult tok_result = tokenize(s);
  TokenizerStatus tok_status = tok_result.status;
  if (tok_status != TOKENIZER_SUCCESS) {
    return (EvalResult){.type = EVAL_ERROR_TOKENIZATION,
                        .tokenizer_error = tok_status};
  }
  ParserResult pr =
      parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
  if (pr.type != PARSE_SUCCESS) {
    return (EvalResult){.type = EVAL_ERROR_PARSING, .parser_error = pr.type};
  }
  return eval_expr_list(pr.expr);
}
