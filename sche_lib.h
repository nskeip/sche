#ifndef _SCHE_LIB_H
#define _SCHE_LIB_H

#include <stddef.h>

typedef union {
  const char *s;
  long num;
} Value;

typedef struct {
  enum TokenType {
    TOKEN_TYPE_SYMBOL = 0,
    TOKEN_TYPE_NUMBER = 1,
    TOKEN_TYPE_PAR_OPEN = 2,
    TOKEN_TYPE_PAR_CLOSE = 3,
  } type;
  Value value;
} Token;

typedef struct {
  size_t tokens_n;
  Token *tokens;
} TokenList;

void token_list_free(TokenList *);
TokenList tokenize(const char *);

typedef struct ExpressionT {
  enum expression_type {
    EXPR_TYPE_NAME = 1,
    EXPR_TYPE_INT = 2,
    EXPR_TYPE_SUBEXPR = 3,
  } type;
  union {
    Value value;
    struct ExpressionT *subexpr;
  };
  struct ExpressionT *next;
} Expression;

void expression_free(Expression *);

Expression *parse(TokenList);

typedef struct {
  const char *name;
  long (*const run)(int, int);
} Function;

int eval_expr_list(const Expression *); // TODO: rename to eval_expr
int eval(const char *);
#endif // !_SCHE_LIB_H
