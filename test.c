#include "sche_lib.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  {
    TokenList token_list = tokenize("(foo(bar 42 99baz) )");
    Token *tokens = token_list.tokens;
    assert(token_list.tokens_n == 8);
    assert(tokens[0].type == TOKEN_TYPE_PAR_OPEN);
    assert(tokens[1].type == TOKEN_TYPE_SYMBOL);
    assert(strncmp(tokens[1].value.s, "foo", 3) == 0);

    assert(tokens[2].type == TOKEN_TYPE_PAR_OPEN);

    assert(tokens[3].type == TOKEN_TYPE_SYMBOL);
    assert(strncmp(tokens[3].value.s, "bar", 3) == 0);

    assert(tokens[4].type == TOKEN_TYPE_NUMBER);
    assert(tokens[4].value.num == 42);

    // print token type:
    assert(tokens[5].type == TOKEN_TYPE_SYMBOL);
    assert(strncmp(tokens[5].value.s, "99baz", 5) == 0);

    assert(tokens[6].type == TOKEN_TYPE_PAR_CLOSE);
    assert(tokens[7].type == TOKEN_TYPE_PAR_CLOSE);
    token_list_free(&token_list);
  }
  {
    const size_t test_buffer_size = 2048;
    char many_tokens[test_buffer_size];
    memset(many_tokens, '(', test_buffer_size - 1);
    many_tokens[test_buffer_size - 1] = '\0';

    TokenList token_list = tokenize(many_tokens);
    assert(token_list.tokens_n == 2047);
    token_list_free(&token_list);
  }
  {
    Expression *expr = parse(tokenize("(+ 1 2)"));
    assert(expr != NULL);
    assert(expr->type == EXPR_TYPE_SUBEXPR);
    assert(strncmp(expr->value.s, "+", 1) == 0);

    assert(expr->next->type == EXPR_TYPE_INT);
    assert(expr->next->value.num == 1);

    assert(expr->next->next->type == EXPR_TYPE_INT);
    assert(expr->next->next->value.num == 2);
    assert(expr->next->next->next == NULL);

    /* EvalResult er = eval_expr_list(pr.expr); */
    /* assert(er.type == EVAL_SUCCESS); */
    /* assert(er.value.num == 3); */
  }
  /*
  {
    TokenizerResult tok_result = tokenize("(+ (- 5 4) 40 1)");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    ParserResult pr =
        parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
    assert(pr.type == PARSE_SUCCESS);
    assert(pr.expr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->value.s == '+');

    assert(pr.expr->next != NULL);
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 4);
    assert(pr.expr->next->subexpr->next->next->next == NULL);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);
    assert(pr.expr->next->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->next->value.num == 1);
    assert(pr.expr->next->next->next->next == NULL);

    EvalResult er = eval_expr_list(pr.expr);
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 42);

    // and nothing changed after evaluation
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 4);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);
    assert(pr.expr->next->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->next->value.num == 1);
  }
  */
  printf("\x1b[32m"); // green text
  printf("\u2713 ");  // Unicode check mark
  printf("\x1b[0m");  // Reset text color to default
  printf("All tests passed\n");
}
