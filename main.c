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

void reportError(ErrorType et, unsigned int line_no, unsigned int char_no) {
  switch (et) {
  case UnknownIdentifier:
    printf("Unknown identifier at line %d, char %d", line_no, char_no);
    break;
  case ZeroDivisionError:
    printf("Hey! Do not divide by zero at line %d, char %d", line_no, char_no);
    break;
  default:
    puts("Unknown error");
    break;
  }
}

typedef struct {
  bool ok;
  union {
    ErrorType error_type;
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
