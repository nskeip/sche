#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { Number, Symbol, ParenOpen, ParenClose } TokenType;

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

#define MAX_TOKENS 1024

typedef enum { TooManyTokens, SymbolWithDigitsInBeginning } TokenizerErrorType;

typedef struct {
  bool ok;
  size_t tokens_n;
  union {
    Token tokens[MAX_TOKENS];
    struct {
      TokenizerErrorType type;
      size_t line_no;
      size_t char_no;
    } error;
  };
} TokenizerResult;

static TokenizerResult mk_error_token_result(TokenizerErrorType type,
                                             size_t line_no, size_t char_no) {
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

TokenizerResult tokenize(char *s) {
  TokenizerResult result = {.ok = true, .tokens = {{0}}};
  size_t token_count = 0;
  size_t line_no = 0;
  int sign = 1;
  for (size_t char_no = 0; *s != '\0'; ++s, ++char_no) {
    if (token_count == MAX_TOKENS) {
      return mk_error_token_result(TooManyTokens, line_no, char_no);
    }
    char c = *s;
    if (isspace(c)) {
      if (c == '\n') {
        ++line_no;
        char_no = 0;
      }
      continue;
    } else if (isdigit(c)) {
      result.tokens[token_count].type = Number;
      result.tokens[token_count].value.i = sign * atoi(s);
      sign = 1;
      while (isdigit(*(s + 1))) {
        ++s;
        ++char_no;
      }
      if (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        return mk_error_token_result(SymbolWithDigitsInBeginning, line_no,
                                     char_no);
      }
    } else if (c == '-' && isdigit(*(s + 1))) {
      sign = -1;
      continue;
    } else if (c == '(') {
      result.tokens[token_count].type = ParenOpen;
    } else if (c == ')') {
      result.tokens[token_count].type = ParenClose;
    } else {
      result.tokens[token_count].type = Symbol;

      char *position_of_name_beginning = s;
      result.tokens[token_count].value.s.arr = position_of_name_beginning;
      while (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        ++s;
        ++char_no;
      }
      result.tokens[token_count].value.s.chars_n =
          s - position_of_name_beginning + 1;
    }

    ++token_count;
  }
  result.tokens_n = token_count;
  return result;
}

typedef enum { Literal, Call } ExpressionType;

typedef struct Expression {
  ExpressionType type;
  union {
    int i;
    struct {
      CharBuff name;
      size_t params_n;
      struct Expression **params;
    } f_call;
  } value;
} Expression;

Expression parse(size_t tokens_n, Token tokens[]) {
  Expression result;
  for (size_t i = 0; i < tokens_n; ++i) {
  }
  return result;
}

int main(void) {
  {
    const size_t test_size = MAX_TOKENS;
    char many_tokens[test_size];
    memset(many_tokens, '(', test_size);
    TokenizerResult tr = tokenize(many_tokens);
    assert(tr.ok);
    assert(tr.tokens_n == MAX_TOKENS);
  }
  {
    const size_t test_size = MAX_TOKENS + 1;
    char too_many_tokens[test_size];
    memset(too_many_tokens, '(', test_size);
    TokenizerResult tr = tokenize(too_many_tokens);
    assert(!tr.ok);
    assert(tr.error.type == TooManyTokens);
    assert(tr.error.char_no == MAX_TOKENS);
  }
  {
    TokenizerResult tr = tokenize("(+ -1 20)");
    assert(tr.ok);
    assert(tr.tokens_n == 5);

    assert(tr.tokens[0].type == ParenOpen);

    assert(tr.tokens[1].type == Symbol);
    assert(tr.tokens[1].value.s.chars_n == 1);
    assert(*tr.tokens[1].value.s.arr == '+');

    assert(tr.tokens[2].type == Number);
    assert(tr.tokens[2].value.i == -1);

    assert(tr.tokens[3].type == Number);
    assert(tr.tokens[3].value.i == 20);

    assert(tr.tokens[4].type == ParenClose);
  }
  {
    TokenizerResult tr = tokenize("99c");
    assert(!tr.ok);
    assert(tr.error.type == SymbolWithDigitsInBeginning);
    assert(tr.error.line_no == 0);
    assert(tr.error.char_no == 1);
  }
  {
    TokenizerResult tr = tokenize("e2e4 abc");
    assert(tr.ok);
    assert(tr.tokens_n == 2);

    assert(tr.tokens[0].type == Symbol);
    assert(tr.tokens[0].value.s.chars_n == 4);
    assert(strncmp(tr.tokens[0].value.s.arr, "e2e4", 4) == 0);

    assert(tr.tokens[1].type == Symbol);
    assert(tr.tokens[1].value.s.chars_n == 3);
    assert(strncmp(tr.tokens[1].value.s.arr, "abc", 3) == 0);
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
  return EXIT_SUCCESS;
}
