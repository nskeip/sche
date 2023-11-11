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

#define MAX_TOKENS 1024

typedef struct {
  bool ok;
  union {
    Token tokens[MAX_TOKENS];
    TokenizerError error;
  };
} TokenizerResult;

TokenizerResult tokenize(const char *s) {
  TokenizerResult result;
  for (size_t i = 0; *s != '\0' && i < MAX_TOKENS; ++s, ++i) {
    printf("%zu) %c\n", i, *s);
  }
  return result;
}

int main(void) {
  TokenizerResult tr = tokenize("(+ 1 2)");
  assert(tr.error == UnknownIdentifier);
  return EXIT_SUCCESS;
}
