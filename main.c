#include "memory_tracker.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_NAME,
  TOKEN_TYPE_PAR_OPEN,
  TOKEN_TYPE_PAR_CLOSE
} TokenType;

typedef union {
  const char *s;
  long num;
} Value;

typedef struct {
  TokenType type;
  Value value;
} Token;

typedef enum {
  TOKENIZER_SUCCESS,
  TOKENIZER_ERROR_TOO_MANY_TOKENS,
  TOKENIZER_ERROR_INVALID_NAME,
  TOKENIZER_ERROR_MEMORY_ALLOC
} TokenizerStatus;

typedef struct {
  size_t tokens_n;
  Token *tokens;
} TokenList;

typedef struct {
  TokenizerStatus status;
  TokenList token_list;
} TokenizerResult;

static MemoryTracker *mt;
static void *memory_tracker_allocate(size_t size) {
  return memory_tracker_push(mt, size);
}
static void my_release_all() { memory_tracker_release(mt); }
#define ALLOCATOR memory_tracker_allocate
#define DEALLOCATOR my_release_all

typedef void *(*allocator)(size_t);

TokenizerResult tokenize_with_allocator(const char *s, allocator alloc) {
  TokenizerResult result = {.status = TOKENIZER_SUCCESS,
                            .token_list = {.tokens_n = 0, .tokens = NULL}};

  char *token, *string, *tofree;
  tofree = string = strdup(s);
  assert(tofree != NULL);

  MemoryTracker *tmp_tokens = memory_tracker_init(4096);
  while ((token = strsep(&string, " \t\r\n")) != NULL) { // code by words
    while (*token != '\0') {                             // word by characters
      ++result.token_list.tokens_n;
      Token *new_token = memory_tracker_push(tmp_tokens, sizeof(Token));
      assert(new_token != NULL);
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
                 *end_of_number != '\0') { // digits + non-digits
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
  for (size_t i = 0; i < result.token_list.tokens_n; ++i) {
    result.token_list.tokens[i] = *((Token *)tmp_tokens->pointers[i]);
  }
cleanup:
  memory_tracker_release(tmp_tokens);
  free(tofree);
  return result;
}

TokenizerResult tokenize(const char *s) {
  return tokenize_with_allocator(s, ALLOCATOR);
}

typedef enum {
  EXPR_TYPE_NAME,
  EXPR_TYPE_INT,
  EXPR_TYPE_SUBEXPR
} ExpressionType;

typedef struct Expression {
  ExpressionType type;
  union {
    Value value;
    struct Expression *subexpr;
  };
  struct Expression *next;
} Expression;

typedef enum {
  PARSE_SUCCESS,
  PARSE_ERROR_TOO_SHORT_EXPR,
  PARSE_ERROR_UNBALANCED_PAR
} ParserResultType;

typedef struct {
  ParserResultType type;
  Expression *expr;
} ParserResult;

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

typedef enum {
  EVAL_SUCCESS,
  EVAL_ERROR_TOKENIZATION,
  EVAL_ERROR_PARSING,
  EVAL_ERROR_NAME_EXPECTED,
  EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED,
  EVAL_ERROR_DIVISION_BY_ZERO,
  EVAL_ERROR_UNDEFINED_FUNCTION
} EvalResultType;

typedef struct {
  EvalResultType type;
  union {
    TokenizerStatus tokenizer_error;
    ParserResultType parser_error;
    Value value;
  };
} EvalResult;

EvalResult eval_expr_list(const Expression *expr) {
  {
    bool expression_begins_with_name = expr->type == EXPR_TYPE_SUBEXPR;
    if (!expression_begins_with_name) {
      return (EvalResult){.type = EVAL_ERROR_NAME_EXPECTED};
    }
  }

  int a;
  if (expr->next->type == EXPR_TYPE_INT) {
    a = expr->next->value.num;
  } else if (expr->next->type == EXPR_TYPE_SUBEXPR) {
    EvalResult subexpr_eval_result = eval_expr_list(expr->next->subexpr);
    if (subexpr_eval_result.type != EVAL_SUCCESS) {
      return subexpr_eval_result;
    }
    a = subexpr_eval_result.value.num;
  } else {
    return (EvalResult){.type = EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED};
  }

  int b;
  if (expr->next->next->type == EXPR_TYPE_INT) {
    b = expr->next->next->value.num;
  } else if (expr->next->next->type == EXPR_TYPE_SUBEXPR) {
    EvalResult subexpr_eval_result = eval_expr_list(expr->next->next->subexpr);
    if (subexpr_eval_result.type != EVAL_SUCCESS) {
      return subexpr_eval_result;
    }
    b = subexpr_eval_result.value.num;
  } else {
    return (EvalResult){.type = EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED};
  }

  EvalResult result = {.type = EVAL_SUCCESS};
  switch (*expr->value.s) {
  case '+':
    result.value.num = a + b;
    break;
  case '-':
    result.value.num = a - b;
    break;
  case '*':
    result.value.num = a * b;
    break;
  case '/': {
    if (b == 0) {
      return (EvalResult){.type = EVAL_ERROR_DIVISION_BY_ZERO};
    }
    result.value.num = a / b;
    break;
  }
  case '%':
    result.value.num = a % b;
    break;
  default:
    return (EvalResult){.type = EVAL_ERROR_UNDEFINED_FUNCTION};
  }
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

void run_tests(void) {
  mt = memory_tracker_init(1024);
  {
    const size_t test_size = 2048;

    char many_tokens[test_size];
    memset(many_tokens, '(', test_size - 1);
    many_tokens[test_size - 1] = '\0';

    TokenizerResult tok_result = tokenize(many_tokens);
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2047);
  }
  {
    TokenizerResult tok_result = tokenize("(+ -1 20)");
    TokenizerStatus tok_status = tok_result.status;
    assert(tok_status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 5);

    assert(tok_result.token_list.tokens[0].type == TOKEN_TYPE_PAR_OPEN);

    assert(tok_result.token_list.tokens[1].type == TOKEN_TYPE_NAME);
    assert(*tok_result.token_list.tokens[1].value.s == '+');

    assert(tok_result.token_list.tokens[2].type == TOKEN_TYPE_NUMBER);
    assert(tok_result.token_list.tokens[2].value.num == -1);

    assert(tok_result.token_list.tokens[3].type == TOKEN_TYPE_NUMBER);
    assert(tok_result.token_list.tokens[3].value.num == 20);

    assert(tok_result.token_list.tokens[4].type == TOKEN_TYPE_PAR_CLOSE);
  }
  {
    TokenizerResult tok_result = tokenize("99c");
    assert(tok_result.status == TOKENIZER_ERROR_INVALID_NAME);
  }
  {
    TokenizerResult tok_result = tokenize("e2e4 abc");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2);

    assert(tok_result.token_list.tokens[0].type == TOKEN_TYPE_NAME);
    assert(strncmp(tok_result.token_list.tokens[0].value.s, "e2e4", 4) == 0);

    assert(tok_result.token_list.tokens[1].type == TOKEN_TYPE_NAME);
    assert(strncmp(tok_result.token_list.tokens[1].value.s, "abc", 3) == 0);
  }
  {
    assert(tokenize("").status == TOKENIZER_SUCCESS);
    assert(tokenize("my-variable").status == TOKENIZER_SUCCESS);
    assert(tokenize("*special*").status == TOKENIZER_SUCCESS);
    assert(tokenize("+special+").status == TOKENIZER_SUCCESS);
    assert(tokenize("/divide/").status == TOKENIZER_SUCCESS);
    assert(tokenize("=eq=").status == TOKENIZER_SUCCESS);
    assert(tokenize("variable_with_underscore").status == TOKENIZER_SUCCESS);
  }
  {
    TokenizerResult tok_result = tokenize("");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 0);
    assert(parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens)
               .type == PARSE_ERROR_TOO_SHORT_EXPR);
  }
  {
    TokenizerResult tok_result = tokenize("()");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2);
    assert(parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens)
               .type == PARSE_ERROR_TOO_SHORT_EXPR);
  }
  {
    TokenizerResult tok_result = tokenize("(+ 1 2)");
    TokenizerStatus tok_status = tok_result.status;
    assert(tok_status == TOKENIZER_SUCCESS);

    ParserResult pr =
        parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
    assert(pr.type == PARSE_SUCCESS);
    assert(pr.expr->type == EXPR_TYPE_SUBEXPR);
    assert(strncmp(pr.expr->value.s, "+", 1) == 0);

    assert(pr.expr->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->value.num == 1);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 2);
    assert(pr.expr->next->next->next == NULL);

    EvalResult er = eval_expr_list(pr.expr);
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 3);
  }
  {
    EvalResult er = eval("(+ 22 20)");
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 42);
  }
  {
    TokenizerResult tok_result = tokenize("(+ (- 5 3) 40)");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    ParserResult pr =
        parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
    assert(pr.type == PARSE_SUCCESS);
    assert(pr.expr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->value.s == '+');

    assert(pr.expr->next != NULL);
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 3);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);

    EvalResult er = eval_expr_list(pr.expr);
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 42);

    // and nothing changed after evaluation
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 3);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);
  }
  DEALLOCATOR();
  printf("\x1b[32m"); // green text
  printf("\u2713 ");  // Unicode check mark
  printf("\x1b[0m");  // Reset text color to default
  printf("All tests passed\n");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("__   __   _        _                _   _");
    puts("\\ \\ / /__| |_     / \\   _ __   ___ | |_| |__   ___ _ __");
    puts(" \\ V / _ \\ __|   / _ \\ | '_ \\ / _ \\| __| '_ \\ / _ \\ '__|");
    puts("  | |  __/ |_   / ___ \\| | | | (_) | |_| | | |  __/ |");
    puts("  |_|\\___|\\__| /_/   \\_\\_| |_|\\___/ \\__|_| |_|\\___|_|");
    puts("");
    puts("          _           __            __");
    puts(" ___  ___| |__   ___ / / __ ___   __\\ \\");
    puts("/ __|/ __| '_ \\ / _ \\ | '_ ` _ \\ / _ \\ |");
    puts("\\__ \\ (__| | | |  __/ | | | | | |  __/ |");
    puts("|___/\\___|_| |_|\\___| |_| |_| |_|\\___|_|");
    puts("                     \\_\\            /_/");
    puts(" ___       _                           _");
    puts("|_ _|_ __ | |_ ___ _ __ _ __  _ __ ___| |_ ___ _ __");
    puts(" | || '_ \\| __/ _ \\ '__| '_ \\| '__/ _ \\ __/ _ \\ '__|");
    puts(" | || | | | ||  __/ |  | |_) | | |  __/ ||  __/ |");
    puts("|___|_| |_|\\__\\___|_|  | .__/|_|  \\___|\\__\\___|_|");
    puts("                       |_|");
    puts("");
    puts("Usage:");
    printf("%15s    %s\n", "-t, --tests", "Run tests");
    printf("%15s    %s\n", "-c expr", "Evaluate expression (in quotes)");
    puts("");
    return 1;
  }
  if (strcmp(argv[1], "--test") == 0 || strcmp(argv[1], "-t") == 0) {
    run_tests();
    return 0;
  }
  mt = memory_tracker_init(4096);
  if (strcmp(argv[1], "-c") == 0) {
    if (argc < 3) {
      puts("No expression provided");
      goto error_and_clean_up;
    }
    if (argc > 3) {
      puts("Too many arguments");
      goto error_and_clean_up;
    }
    EvalResult eval_result = eval(argv[2]);
    switch (eval_result.type) {
    case EVAL_SUCCESS: {
      printf("%ld\n", eval_result.value.num);
      goto success_and_clean_up;
    }
    case EVAL_ERROR_TOKENIZATION:
      puts("Error tokenizing expression");
      break;
    case EVAL_ERROR_PARSING:
      puts("Error parsing expression");
      break;
    case EVAL_ERROR_NAME_EXPECTED:
      puts("Named expression expected");
      break;
    case EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED:
      puts("Integer or subexpression expected");
      break;
    case EVAL_ERROR_DIVISION_BY_ZERO:
      puts("Division by zero! You are a bad person!");
      break;
    case EVAL_ERROR_UNDEFINED_FUNCTION:
      puts("Error evaluating expression");
      break;
    }
    goto error_and_clean_up;
  }
success_and_clean_up:
  memory_tracker_release(mt);
  return EXIT_SUCCESS;

error_and_clean_up:
  memory_tracker_release(mt);
  return EXIT_FAILURE;
}
