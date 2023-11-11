#include <assert.h>
#include <stdbool.h>
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

typedef enum { UnknownIdentifier } TokenizerError;

void reportTokenizerError(TokenizerError e, unsigned int line_no,
                          unsigned int char_no) {
  switch (e) {
  case UnknownIdentifier:
    printf("Unknown identifier at line %d, char %d", line_no, char_no);
    break;
  default:
    puts("Unknown error");
    break;
  }
}

typedef struct {
  bool ok;
  union {
    Token *tokens;
    TokenizerError error;
  };
} TokenizerResult;

TokenizerResult tokenize(const char *s) {
  TokenizerResult result = {.ok = false, .error = UnknownIdentifier};
  return result;
}

int main(void) {
  TokenizerResult tr = tokenize("(+ 1 2)");
  assert(tr.error == UnknownIdentifier);
  return EXIT_SUCCESS;
}
