#include "memory_tracker.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  NumberToken,
  NameToken,
  ParenOpenToken,
  ParenCloseToken
} TokenType;

typedef struct {
  size_t chars_n;
  const char *arr;
} CharBuff;

typedef union {
  CharBuff s;
  int i;
} Value;

typedef struct {
  TokenType type;
  Value value;
} Token;

typedef enum {
  TooManyTokensTokenizerError,
  NameWithDigitsInBeginningTokenizerError
} TokenizerErrorType;

typedef struct {
  bool ok;
  size_t tokens_n;
  union {
    Token *tokens_sequence;
    struct {
      TokenizerErrorType type;
      size_t line_no;
      size_t char_no;
    } error;
  };
} TokenizerResult;

static bool is_valid_right_limiter_of_name_or_number(char c) {
  // if `c` is the next right of some number or a name, it means
  // that the parsing of the number or name should be finished
  return c == '\0' || c == '(' || c == ')' || isspace(c);
}

static MemoryTracker *mt;
static void *my_allocate(size_t size) { return memory_tracker_push(mt, size); }
static void my_release() { memory_tracker_release(mt); }

TokenizerResult tokenize(const char *s) {
  size_t line_no = 0;
  int sign = 1;
  TokenizerResult result = {.ok = true, .tokens_n = 0};
  MemoryTracker *tmp_tokens = memory_tracker_init(4096);
  for (size_t char_no = 0; *s != '\0'; ++s, ++char_no) {
    char c = *s;
    if (isspace(c)) {
      if (c == '\n') {
        ++line_no;
        char_no = 0;
      }
      continue;
    } else if (c == '-' && isdigit(*(s + 1))) {
      sign = -1;
      continue;
    }

    Token *new_token = memory_tracker_push(tmp_tokens, sizeof(Token));
    assert(new_token != NULL);
    result.tokens_n++;
    if (isdigit(c)) {
      new_token->type = NumberToken;
      new_token->value.i = sign * atoi(s);
      sign = 1;
      while (isdigit(*(s + 1))) {
        ++s;
        ++char_no;
      }
      if (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        memory_tracker_release(tmp_tokens);
        return (TokenizerResult){.ok = false,
                                 .error.type =
                                     NameWithDigitsInBeginningTokenizerError,
                                 .error.line_no = line_no,
                                 .error.char_no = char_no};
      }
    } else if (c == '(') {
      new_token->type = ParenOpenToken;
    } else if (c == ')') {
      new_token->type = ParenCloseToken;
    } else {
      new_token->type = NameToken;

      const char *position_of_name_beginning = s;
      new_token->value.s.arr = position_of_name_beginning;
      while (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        ++s;
        ++char_no;
      }
      new_token->value.s.chars_n = s - position_of_name_beginning + 1;
    }
  }
  result.tokens_sequence = my_allocate(sizeof(Token) * result.tokens_n);
  for (size_t i = 0; i < result.tokens_n; ++i) {
    result.tokens_sequence[i] = *((Token *)tmp_tokens->pointers[i]);
  }
  memory_tracker_release(tmp_tokens);
  return result;
}

typedef enum {
  NamedExpressionType,
  IntExpressionType,
  SubExpressionType
} ExpressionType;

typedef struct ExpressionsList {
  struct {
    ExpressionType type;
    union {
      Value value;
      struct ExpressionsList *subexpr;
    };
  } head;
  struct ExpressionsList *tail;
} ExpressionsList;

typedef enum {
  TooShortExpressionParseError,
  CheckParensParseError
} ParserErrorType;

typedef struct {
  bool ok;
  union {
    ExpressionsList values_list;
    ParserErrorType error_type;
  };
} ParserResult;

ParserResult parse(size_t tokens_n, const Token tokens[]) {
  if (tokens_n > 0 && (tokens[0].type != ParenOpenToken ||
                       tokens[tokens_n - 1].type != ParenCloseToken)) {
    return (ParserResult){.ok = false, .error_type = CheckParensParseError};
  }
  if (tokens_n < 3) {
    return (ParserResult){.ok = false,
                          .error_type = TooShortExpressionParseError};
  }
  ParserResult result = {.ok = true};
  ExpressionsList *current_values_list = &result.values_list;
  const size_t idx_of_closing_par = tokens_n - 1;
  for (size_t i = 1; i < idx_of_closing_par; ++i) {
    switch (tokens[i].type) {
    case NumberToken:
      current_values_list->head.type = IntExpressionType;
      current_values_list->head.value.i = tokens[i].value.i;
      break;
    case NameToken:
      current_values_list->head.type = NamedExpressionType;
      current_values_list->head.value.s = tokens[i].value.s;
      break;
    case ParenOpenToken: {
      current_values_list->head.type = SubExpressionType;
      size_t subexpr_tokens_n = 1;
      {
        size_t opening_parenthesis_n = 1;
        while (opening_parenthesis_n != 0) {
          if (tokens[i + subexpr_tokens_n].type == ParenOpenToken) {
            ++opening_parenthesis_n;
          } else if (tokens[i + subexpr_tokens_n].type == ParenCloseToken) {
            --opening_parenthesis_n;
          }
          ++subexpr_tokens_n;
        }
      }
      ParserResult subexpr_parse_result = parse(subexpr_tokens_n, tokens + i);
      if (!subexpr_parse_result.ok) {
        return (ParserResult){.ok = false,
                              .error_type = subexpr_parse_result.error_type};
      }
      current_values_list->head.subexpr = &subexpr_parse_result.values_list;
      i += subexpr_tokens_n - 1;
      break;
    }
    case ParenCloseToken:
      assert(false);
    }

    const bool we_have_some_tokens_left = i != idx_of_closing_par - 1;
    if (we_have_some_tokens_left) {
      current_values_list->tail = my_allocate(sizeof(ExpressionsList));
      current_values_list->tail->tail = NULL;
      current_values_list = current_values_list->tail;
    }
  }
  return result;
}

typedef enum {
  SuccessfulEval,
  TokenizationWhileEvalError,
  ParsingWhileEvalError,
  NamedExpressionExpectedEvalError,
  IntegerOrSubExpressionExpectedEvalError,
  ZeroDivisionEvalError,
  UndefinedFunctionEvalError
} EvalStatus;

typedef struct {
  EvalStatus status;
  union {
    TokenizerErrorType tokenizer_error;
    ParserErrorType parser_error;
    Value value;
  };
} EvalResult;

EvalResult eval_expr_list(const ExpressionsList exprs) {
  if (exprs.head.type != NamedExpressionType) {
    return (EvalResult){.status = NamedExpressionExpectedEvalError};
  }

  int a;
  if (exprs.tail->head.type == IntExpressionType) {
    a = exprs.tail->head.value.i;
  } else if (exprs.tail->head.type == SubExpressionType) {
    EvalResult subexpr_eval_result = eval_expr_list(*exprs.tail->head.subexpr);
    if (subexpr_eval_result.status != SuccessfulEval) {
      return subexpr_eval_result;
    }
    a = subexpr_eval_result.value.i;
  } else {
    return (EvalResult){.status = IntegerOrSubExpressionExpectedEvalError};
  }

  int b;
  if (exprs.tail->tail->head.type == IntExpressionType) {
    b = exprs.tail->tail->head.value.i;
  } else if (exprs.tail->tail->head.type == SubExpressionType) {
    EvalResult subexpr_eval_result =
        eval_expr_list(*exprs.tail->tail->head.subexpr);
    if (subexpr_eval_result.status != SuccessfulEval) {
      return subexpr_eval_result;
    }
    b = subexpr_eval_result.value.i;
  } else {
    return (EvalResult){.status = IntegerOrSubExpressionExpectedEvalError};
  }

  EvalResult result = {.status = SuccessfulEval};
  switch (*exprs.head.value.s.arr) {
  case '+':
    result.value.i = a + b;
    break;
  case '-':
    result.value.i = a - b;
    break;
  case '*':
    result.value.i = a * b;
    break;
  case '/': {
    if (b == 0) {
      return (EvalResult){.status = ZeroDivisionEvalError};
    }
    result.value.i = a / b;
    break;
  }
  case '%':
    result.value.i = a % b;
    break;
  default:
    return (EvalResult){.status = UndefinedFunctionEvalError};
  }
  return result;
}

EvalResult eval(const char *s) {
  TokenizerResult tr = tokenize(s);
  if (!tr.ok) {
    return (EvalResult){.status = TokenizationWhileEvalError,
                        .tokenizer_error = tr.error.type};
  }
  ParserResult pr = parse(tr.tokens_n, tr.tokens_sequence);
  if (!pr.ok) {
    return (EvalResult){.status = ParsingWhileEvalError,
                        .parser_error = pr.error_type};
  }
  return eval_expr_list(pr.values_list);
}

void run_tests(void) {
  mt = memory_tracker_init(1024);
  {
    const size_t test_size = 2048;

    char many_tokens[test_size];
    memset(many_tokens, '(', test_size - 1);
    many_tokens[test_size - 1] = '\0';

    TokenizerResult tr = tokenize(many_tokens);
    assert(tr.ok);
    assert(tr.tokens_n == 2047);
  }
  {
    TokenizerResult tr = tokenize("(+ -1 20)");
    assert(tr.ok);
    assert(tr.tokens_n == 5);

    assert(tr.tokens_sequence[0].type == ParenOpenToken);

    assert(tr.tokens_sequence[1].type == NameToken);
    assert(tr.tokens_sequence[1].value.s.chars_n == 1);
    assert(*tr.tokens_sequence[1].value.s.arr == '+');

    assert(tr.tokens_sequence[2].type == NumberToken);
    assert(tr.tokens_sequence[2].value.i == -1);

    assert(tr.tokens_sequence[3].type == NumberToken);
    assert(tr.tokens_sequence[3].value.i == 20);

    assert(tr.tokens_sequence[4].type == ParenCloseToken);
  }
  {
    TokenizerResult tr = tokenize("99c");
    assert(!tr.ok);
    assert(tr.error.type == NameWithDigitsInBeginningTokenizerError);
    assert(tr.error.line_no == 0);
    assert(tr.error.char_no == 1);
  }
  {
    TokenizerResult tr = tokenize("e2e4 abc");
    assert(tr.ok);
    assert(tr.tokens_n == 2);

    assert(tr.tokens_sequence[0].type == NameToken);
    assert(tr.tokens_sequence[0].value.s.chars_n == 4);
    assert(strncmp(tr.tokens_sequence[0].value.s.arr, "e2e4", 4) == 0);

    assert(tr.tokens_sequence[1].type == NameToken);
    assert(tr.tokens_sequence[1].value.s.chars_n == 3);
    assert(strncmp(tr.tokens_sequence[1].value.s.arr, "abc", 3) == 0);
  }
  {
    assert(tokenize("").ok);
    assert(tokenize("my-variable").ok);
    assert(tokenize("*special*").ok);
    assert(tokenize("+special+").ok);
    assert(tokenize("/divide/").ok);
    assert(tokenize("=eq=").ok);
    assert(tokenize("variable_with_underscore").ok);
  }
  {
    TokenizerResult tr = tokenize("");
    assert(tr.ok);
    assert(tr.tokens_n == 0);
    assert(!parse(tr.tokens_n, tr.tokens_sequence).ok);
  }
  {
    TokenizerResult tr = tokenize("()");
    assert(tr.ok);
    assert(tr.tokens_n == 2);
    assert(!parse(tr.tokens_n, tr.tokens_sequence).ok);
  }
  {
    TokenizerResult tr = tokenize("(+ 1 2)");
    assert(tr.ok);

    ParserResult pr = parse(tr.tokens_n, tr.tokens_sequence);
    assert(pr.ok);
    assert(pr.values_list.head.type == NamedExpressionType);
    assert(pr.values_list.head.value.s.chars_n == 1);
    assert(strncmp(pr.values_list.head.value.s.arr, "+", 1) == 0);

    assert(pr.values_list.tail->head.type == IntExpressionType);
    assert(pr.values_list.tail->head.value.i == 1);

    assert(pr.values_list.tail->tail->head.type == IntExpressionType);
    assert(pr.values_list.tail->tail->head.value.i == 2);
    assert(pr.values_list.tail->tail->tail == NULL);

    EvalResult er = eval_expr_list(pr.values_list);
    assert(er.status == SuccessfulEval);
    assert(er.value.i == 3);
  }
  {
    EvalResult er = eval("(+ 22 20)");
    assert(er.status == SuccessfulEval);
    assert(er.value.i == 42);
  }
  {
    TokenizerResult tr = tokenize("(+ (- 5 3) 40)");
    assert(tr.ok);
    ParserResult pr = parse(tr.tokens_n, tr.tokens_sequence);
    assert(pr.ok);
    assert(pr.values_list.head.type == NamedExpressionType);
    assert(*pr.values_list.head.value.s.arr == '+');

    assert(pr.values_list.tail->head.subexpr->head.type == NamedExpressionType);
    assert(*pr.values_list.tail->head.subexpr->head.value.s.arr == '-');
    assert(pr.values_list.tail->head.subexpr->tail->head.value.i == 5);
    assert(pr.values_list.tail->head.subexpr->tail->tail->head.value.i == 3);

    assert(pr.values_list.tail->tail->head.type == IntExpressionType);
    assert(pr.values_list.tail->tail->head.value.i == 40);
  }
  {
    EvalResult er = eval("(+ (- 5 3) 40)");
    assert(er.status == SuccessfulEval);
    assert(er.value.i == 42);
  }
  my_release();
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
    switch (eval_result.status) {
    case SuccessfulEval: {
      printf("%d\n", eval_result.value.i);
      goto success_and_clean_up;
    }
    case TokenizationWhileEvalError:
      puts("Error tokenizing expression");
      break;
    case ParsingWhileEvalError:
      puts("Error parsing expression");
      break;
    case NamedExpressionExpectedEvalError:
      puts("Named expression expected");
      break;
    case IntegerOrSubExpressionExpectedEvalError:
      puts("Integer or subexpression expected");
      break;
    case ZeroDivisionEvalError:
      puts("Division by zero! You are a bad person!");
      break;
    case UndefinedFunctionEvalError:
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
