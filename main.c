#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

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

typedef struct {
  TokenType type;
  union {
    CharBuff s;
    int i;
  } value;
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

static TokenizerResult mk_error_tokenizer_result(TokenizerErrorType type,
                                                 size_t line_no,
                                                 size_t char_no) {
  TokenizerResult err_result = {.ok = false,
                                .error.type = type,
                                .error.line_no = line_no,
                                .error.char_no = char_no};
  return err_result;
}

static bool is_valid_right_limiter_of_name_or_number(char c) {
  // if `c` is the next right of some number or a name, it means
  // that the parsing of the number or name should be finished
  return c == '\0' || c == '(' || c == ')' || isspace(c);
}

static Arena my_arena;
static void *my_allocate(size_t size) {
  return arena_push_dyn(&my_arena, size);
}
static void my_release() { arena_release(&my_arena); }

static Token *allocate_token() { return my_allocate(sizeof(Token)); }

TokenizerResult tokenize(char *s) {
  size_t token_count = 0;
  size_t line_no = 0;
  int sign = 1;
  TokenizerResult result = {.ok = true, .tokens_sequence = NULL};
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

    Token *new_token = allocate_token();
    if (result.tokens_sequence == NULL) {
      result.tokens_sequence = new_token;
    }
    token_count++;
    if (isdigit(c)) {
      new_token->type = NumberToken;
      new_token->value.i = sign * atoi(s);
      sign = 1;
      while (isdigit(*(s + 1))) {
        ++s;
        ++char_no;
      }
      if (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        return mk_error_tokenizer_result(
            NameWithDigitsInBeginningTokenizerError, line_no, char_no);
      }
    } else if (c == '(') {
      new_token->type = ParenOpenToken;
    } else if (c == ')') {
      new_token->type = ParenCloseToken;
    } else {
      new_token->type = NameToken;

      char *position_of_name_beginning = s;
      new_token->value.s.arr = position_of_name_beginning;
      while (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        ++s;
        ++char_no;
      }
      new_token->value.s.chars_n = s - position_of_name_beginning + 1;
    }
  }
  result.tokens_n = token_count;
  return result;
}

typedef enum { NamedExpressionType, IntExpressionType } ExpressionType;

typedef struct ExpressionsList {
  struct {
    ExpressionType value_type;
    union {
      int i;
      CharBuff s;
    };
  } head;
  struct ExpressionsList *tail;
} ExpressionsList;

typedef enum {
  TooShortExpressionParseError,
  CheckParentesisParseError
} ParserErrorType;

typedef struct {
  bool ok;
  union {
    ExpressionsList values_list;
    ParserErrorType error_type;
  };
} ParserResult;

static ParserResult
mk_error_expression_parsing_result(ParserErrorType error_type) {
  ParserResult r = {.ok = false, .error_type = error_type};
  return r;
}

ParserResult parse(size_t tokens_n, Token tokens[]) {
  if (tokens_n > 0 && (tokens[0].type != ParenOpenToken ||
                       tokens[tokens_n - 1].type != ParenCloseToken)) {
    return mk_error_expression_parsing_result(CheckParentesisParseError);
  }
  if (tokens_n < 3) {
    return mk_error_expression_parsing_result(TooShortExpressionParseError);
  }
  ParserResult result = {.ok = true};
  ExpressionsList *current_values_list = &result.values_list;
  const size_t idx_of_closing_par = tokens_n - 1;
  for (size_t i = 1; i < idx_of_closing_par; ++i) {
    switch (tokens[i].type) {
    case NumberToken:
      current_values_list->head.value_type = IntExpressionType;
      current_values_list->head.i = tokens[i].value.i;
      break;
    case NameToken:
      current_values_list->head.value_type = NamedExpressionType;
      current_values_list->head.s = tokens[i].value.s;
      break;
    case ParenOpenToken:
    case ParenCloseToken:
      assert(false);
    }
    if (i != idx_of_closing_par - 1) {
      current_values_list->tail = my_allocate(sizeof(ExpressionsList));
      current_values_list->tail->tail = NULL;
      current_values_list = current_values_list->tail;
    }
  }
  return result;
}

int main(void) {
  my_arena = arena_alloc(1024);
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
    assert(pr.values_list.head.value_type == NamedExpressionType);
    assert(pr.values_list.head.s.chars_n == 1);
    assert(strncmp(pr.values_list.head.s.arr, "+", 1) == 0);

    assert(pr.values_list.tail->head.value_type == IntExpressionType);
    assert(pr.values_list.tail->head.i == 1);

    assert(pr.values_list.tail->tail->head.value_type == IntExpressionType);
    assert(pr.values_list.tail->tail->head.i == 2);
    assert(pr.values_list.tail->tail->tail == NULL);
  }
  my_release();
  return EXIT_SUCCESS;
}
