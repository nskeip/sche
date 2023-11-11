#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum { Number, Op, Paren } TokenType;

typedef struct {
  TokenType type;
  union {
    char c;
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

TokenizerResult tokenize(const char *s) {
  TokenizerResult result = {.ok = true, .tokens = {0}};
  size_t token_count = 0;
  size_t line_i = 0;
  for (size_t char_i = 0; *s != '\0' && token_count < MAX_TOKENS;
       ++s, ++char_i) {
    char c = *s;
    if (isspace(c)) {
      if (c == '\n') {
        ++line_i;
      }
      continue;
    } else if (isdigit(c)) {
      result.tokens[token_count].type = Number;
      result.tokens[token_count].value.i = atoi(s);
      while (isdigit(*(s + 1))) {
        ++s;
      }
    } else if (c == '(' || c == ')') {
      result.tokens[token_count].type = Paren;
      result.tokens[token_count].value.c = c;
    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
      result.tokens[token_count].type = Op;
      result.tokens[token_count].value.c = c;
    } else {
      TokenizerResult err_result = {
          .ok = false, .line_no = line_i, .char_no = char_i};
      return err_result;
    }

    ++token_count;
  }
  result.tokens_n = token_count;
  return result;
}

int main(void) {
  {
    TokenizerResult tr = tokenize("(+ 1 20)");
    assert(tr.ok);
    assert(tr.tokens_n == 5);

    assert(tr.tokens[0].type == Paren);
    assert(tr.tokens[0].value.c == '(');

    assert(tr.tokens[1].type == Op);
    assert(tr.tokens[1].value.c == '+');

    assert(tr.tokens[2].type == Number);
    assert(tr.tokens[2].value.i == 1);

    assert(tr.tokens[3].type == Number);
    assert(tr.tokens[3].value.i == 20);

    assert(tr.tokens[4].type == Paren);
    assert(tr.tokens[4].value.c == ')');
  }
  return EXIT_SUCCESS;
}
