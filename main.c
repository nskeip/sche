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
    TokenizerError error;
    Token token;
  };
} Result;

#define MAX_TOKENS 1000

Token tokens[MAX_TOKENS] = {0};

int main(void) { return EXIT_SUCCESS; }
