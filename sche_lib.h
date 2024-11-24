#ifndef _SCHE_LIB_H
#define _SCHE_LIB_H

#include <stddef.h>
typedef struct {
  enum TokenType {
    TOKEN_TYPE_SYMBOL = 0,
    TOKEN_TYPE_NUMBER = 1,
    TOKEN_TYPE_PAR_OPEN = 2,
    TOKEN_TYPE_PAR_CLOSE = 3,
  } type;
  union {
    char *s;
    int num;
  } value;
} Token;

typedef struct {
  size_t tokens_n;
  Token *tokens;
} TokenList;

void token_list_free(TokenList *);
TokenList tokenize(const char *);

typedef struct Expression {
  enum expression_type {
    EXPR_TYPE_NAME,
    EXPR_TYPE_INT,
    EXPR_TYPE_SUBEXPR,
  } type;
  union {
    struct Expression *subexpr;
  };
  struct expression *next;
} expression_t;
#endif // !_SCHE_LIB_H
