#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

typedef enum { Number, Name, ParenOpen, ParenClose } TokenType;

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

typedef enum { TooManyTokens, NameWithDigitsInBeginning } TokenizerErrorType;

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

static Arena my_arena;
static void *my_allocate(size_t size) {
  return arena_push_dyn(&my_arena, size);
}
static void my_deallocate() { arena_release(&my_arena); }

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
      new_token->type = Number;
      new_token->value.i = sign * atoi(s);
      sign = 1;
      while (isdigit(*(s + 1))) {
        ++s;
        ++char_no;
      }
      if (!is_valid_right_limiter_of_name_or_number(*(s + 1))) {
        return mk_error_token_result(NameWithDigitsInBeginning, line_no,
                                     char_no);
      }
    } else if (c == '(') {
      new_token->type = ParenOpen;
    } else if (c == ')') {
      new_token->type = ParenClose;
    } else {
      new_token->type = Name;

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

// typedef enum { Literal, Call } ExpressionType;
//
// typedef struct Expression {
//   ExpressionType type;
//   union {
//     int i;
//     struct {
//       CharBuff name;
//       size_t params_n;
//       struct Expression **params;
//     } f_call;
//   } value;
// } Expression;
//
// Expression parse(size_t tokens_n, Token tokens[]) {
//   Expression result;
//   for (size_t i = 0; i < tokens_n; ++i) {
//   }
//   return result;
// }

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

    assert(tr.tokens_sequence[0].type == ParenOpen);

    assert(tr.tokens_sequence[1].type == Name);
    assert(tr.tokens_sequence[1].value.s.chars_n == 1);
    assert(*tr.tokens_sequence[1].value.s.arr == '+');

    assert(tr.tokens_sequence[2].type == Number);
    assert(tr.tokens_sequence[2].value.i == -1);

    assert(tr.tokens_sequence[3].type == Number);
    assert(tr.tokens_sequence[3].value.i == 20);

    assert(tr.tokens_sequence[4].type == ParenClose);
  }
  {
    TokenizerResult tr = tokenize("99c");
    assert(!tr.ok);
    assert(tr.error.type == NameWithDigitsInBeginning);
    assert(tr.error.line_no == 0);
    assert(tr.error.char_no == 1);
  }
  {
    TokenizerResult tr = tokenize("e2e4 abc");
    assert(tr.ok);
    assert(tr.tokens_n == 2);

    assert(tr.tokens_sequence[0].type == Name);
    assert(tr.tokens_sequence[0].value.s.chars_n == 4);
    assert(strncmp(tr.tokens_sequence[0].value.s.arr, "e2e4", 4) == 0);

    assert(tr.tokens_sequence[1].type == Name);
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
  my_deallocate();
  return EXIT_SUCCESS;
}
