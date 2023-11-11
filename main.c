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

typedef enum { UnknownIdentifier, ZeroDivisionError } ErrorType;

typedef struct {
  ErrorType errorType;
  unsigned int lineNo;
  unsigned int charNo;
} Error;

void reportError(Error e) {
  switch (e.errorType) {
  case UnknownIdentifier:
    printf("Unknown identifier at line %d, char %d", e.lineNo, e.charNo);
    break;
  case ZeroDivisionError:
    printf("Hey! Do not divide by zero at line %d, char %d", e.lineNo,
           e.charNo);
    break;
  default:
    puts("Unknown error");
    break;
  }
}

typedef struct {
  bool ok;
  union {
    Error error;
    Token token;
  };
} Result;

#define MAX_TOKENS 1000

Token tokens[MAX_TOKENS] = {0};

int main(void) {
  tokens[0] = (Token){.type = Number, .value = 123};
  printf("%d\n", tokens[0].value.i);
  return EXIT_SUCCESS;
}
