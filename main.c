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

#define MAX_TOKENS 1024

typedef struct {
  bool ok;
  Token tokens[MAX_TOKENS];
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
  assert(tr.ok);
  return EXIT_SUCCESS;
}
