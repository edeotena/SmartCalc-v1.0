#include <stdlib.h>

#include "s21_smart_calc.h"
#include "s21_structs/s21_structs.h"

int get_priority(Token op) {
  int res = 0;
  if (op == ADDITION || op == SUBTRACTION) {
    res = 1;
  } else if (op == MULTIPLICATION || op == DIVISION || op == MODULUS) {
    res = 2;
  } else if (op == POWER) {
    res = 3;
  } else if (op == SIN || op == COS || op == TAN || op == ACOS || op == ASIN ||
             op == ATAN || op == SQRT || op == LN || op == LOG ||
             op == UNARY_PLUS || op == UNARY_MINUS) {
    res = 4;
  }
  return res;
}

int handle_bin(queue **res, stack **operations, Token op, int *operands) {
  int code = SUCCESS;
  while (*operations != NULL &&
         get_priority((*operations)->op) >= get_priority(op) &&
         code == SUCCESS) {
    Token p;
    double temp;
    code = pop(operations, &p, &temp);
    if (code == SUCCESS) {
      if (p == ADDITION || p == SUBTRACTION || p == MULTIPLICATION ||
          p == DIVISION || p == POWER || p == MODULUS) {
        --(*operands);
      }
      code = add(res, p, 0);
    }
  }

  if (code == SUCCESS) {
    code = push(operations, op, 0);
  }

  return code;
}

int handle_bracket(queue **res, stack **operations, int *operands) {
  int code = SUCCESS;
  while (*operations != NULL && (*operations)->op != OPENING_BRACKET &&
         code != STOP_QUEUE) {
    Token op;
    double temp;
    code = pop(operations, &op, &temp);
    if (code == SUCCESS) {
      if (op == ADDITION || op == SUBTRACTION || op == MULTIPLICATION ||
          op == DIVISION || op == POWER || op == MODULUS) {
        --(*operands);
      }
      code = add(res, op, 0);
    }
  }
  if (code == SUCCESS) {
    if (*operations == NULL || (*operations)->op != OPENING_BRACKET) {
      code = FAILURE;
    } else {
      Token remove;
      double temp;
      code = pop(operations, &remove, &temp);
    }
  }
  return code;
}

int flush_stack(queue **res, stack **operations, Token op, int *operands) {
  int code = SUCCESS;
  double temp;
  while (*operations != NULL && code == SUCCESS) {
    code = pop(operations, &op, &temp);
    if (code == SUCCESS) {
      if (op != OPENING_BRACKET) {
        if (op == ADDITION || op == SUBTRACTION || op == MULTIPLICATION ||
            op == DIVISION || op == POWER || op == MODULUS) {
          --(*operands);
        }
        code = add(res, op, 0);
      } else {
        code = FAILURE;
      }
    }
  }

  return code;
}

queue *make_rpn(queue *queue_to_rpn, int *code) {
  *code = SUCCESS;
  int operands = 0, scope_open = 1;
  double value;
  Token op;
  *code = get(&queue_to_rpn, &op, &value);
  queue *res = NULL;
  stack *operations = NULL;
  while (*code != FAILURE && *code != STOP_QUEUE) {
    if (op == VALUE || op == X) {
      *code = add(&res, op, value);
      ++operands;
      scope_open = 0;
    } else if (op == SIN || op == COS || op == TAN || op == ACOS ||
               op == ASIN || op == ATAN || op == SQRT || op == LOG ||
               op == LN || op == OPENING_BRACKET || op == UNARY_PLUS ||
               op == UNARY_MINUS) {
      *code = push(&operations, op, 0);
      scope_open = (op == OPENING_BRACKET ? 1 : 0);
    } else if (op == CLOSING_BRACKET) {
      if (scope_open != 1) {
        *code = handle_bracket(&res, &operations, &operands);
        scope_open = 0;
      } else {
        *code = FAILURE;
      }
    } else if (op == ADDITION || op == SUBTRACTION || op == MULTIPLICATION ||
               op == DIVISION || op == POWER || op == MODULUS) {
      *code = handle_bin(&res, &operations, op, &operands);
      scope_open = 0;
    }
    if (*code != FAILURE) {
      *code = get(&queue_to_rpn, &op, &value);
    }
  }
  if (*code == STOP_QUEUE) {
    *code = flush_stack(&res, &operations, op, &operands);
  }
  if (*code == FAILURE || operands != 1) {
    free_queue(&res);
    free_stack(&operations);
  }

  return res;
}