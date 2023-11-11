#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { Number, Name, ParenOpen, ParenClose } TokenType;

typedef struct {
  size_t chars_n;
  char *arr;
} CharBuff;

typedef struct {
  TokenType type;
  union {
    CharBuff s;
    int i;
  } value;
} Token;

#define MAX_TOKENS 1024

typedef struct {
  bool ok;
  size_t tokens_n;
  union {
    Token tokens[MAX_TOKENS];
    struct {
      size_t line_no;
      size_t char_no;
    };
  };
} TokenizerResult;

TokenizerResult tokenize(char *s) {
  TokenizerResult result = {.ok = true, .tokens = {{0}}};
  size_t token_count = 0;
  size_t line_no = 0;
  int sign = 1;
  for (size_t char_no = 0; *s != '\0' && token_count < MAX_TOKENS;
       ++s, ++char_no) {
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
    } else if (c == '(') {
      result.tokens[token_count].type = ParenOpen;
    } else if (c == ')') {
      result.tokens[token_count].type = ParenClose;
    } else if (c == '-' && isdigit(*(s + 1))) {
      sign = -1;
      continue;
    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
      result.tokens[token_count].type = Name;
      result.tokens[token_count].value.s.chars_n = 1;
      result.tokens[token_count].value.s.arr = s;
    } else {
      TokenizerResult err_result = {
          .ok = false, .line_no = line_no, .char_no = char_no};
      return err_result;
    }

    ++token_count;
  }
  result.tokens_n = token_count;
  return result;
}

typedef enum { Literal, Call } ExpressionType;

typedef struct {
  ExpressionType type;
  union {
    int i;
    struct {
      CharBuff name;
      struct Expression **params;
    } f_call;
  } value;
} Expression;

Expression parse(size_t tokens_n, Token tokens[]) {
  Expression result = {.type = Literal, .value.i = 269};
  return result;
}

int main(void) {
  {
    TokenizerResult tr = tokenize("(+ -1 20)");
    assert(tr.ok);
    assert(tr.tokens_n == 5);

    assert(tr.tokens[0].type == ParenOpen);

    assert(tr.tokens[1].type == Name);
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
    assert(tr.line_no == 0);
    assert(tr.char_no == 2);
  }
  return EXIT_SUCCESS;
}
