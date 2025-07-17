#ifndef TYPES_H
#define TYPES_H

#include "values.h"
#include <stdio.h>

typedef Value (*EvalFn)(void *);
typedef struct { EvalFn fn; void *ctx; } Expr;

typedef void (*ActionFn)(void *ctx);
typedef struct { ActionFn fn; void *ctx; } Action;
enum CellField { CELL_X, CELL_Y, CELL_BUSY };
struct FieldCtx {
    Expr        base;
    enum CellField   fld;
};
Expr make_field_expr(Expr base, enum CellField fld);
Expr make_cell_lit(Expr ex, Expr ey, Expr eb);

static inline void ensure_same(Value a, Value b, const char *where)
{
    if (a.type != b.type) {
        fprintf(stderr,"type mismatch (%s)\n", where);
        exit(1);
    }
}

#define MAX_NEST  8

struct NestedAssignCtx {
    char      *root;                
    size_t     depth;
    Expr       idx[MAX_NEST];
    Expr       rhs;
};

Expr make_typecheck_expr(Expr a, Expr b);
const char *value_type_name(ValueType);

#endif
