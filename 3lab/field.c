#include "types.h"
#include <stdlib.h>

static Value eval_field(void *ctx)
{
    struct FieldCtx *c = ctx;
    Value v = c->base.fn(c->base.ctx);

    if (v.type != VALUE_CELL) {
        fprintf(stderr, "not a cell\n");
        exit(1);
    }
    switch (c->fld) {
        case CELL_X:    return make_int(v.cell.x);
        case CELL_Y:    return make_int(v.cell.y);
        case CELL_BUSY: return make_bool(v.cell.busy);
    }
    return make_int(0);
}
struct CellLitCtx { Expr ex, ey, eb; };

Expr make_field_expr(Expr base, enum CellField fld)
{
    struct FieldCtx *c = malloc(sizeof *c);
    c->base = base;
    c->fld  = fld;
    return (Expr){ eval_field, c };
}

static Value eval_cell_lit(void *ctx)
{
    struct CellLitCtx *c = ctx;
    long long x = c->ex.fn(c->ex.ctx).i;
    long long y = c->ey.fn(c->ey.ctx).i;
    int       b = c->eb.fn(c->eb.ctx).i;
    return make_cell_xyz(x, y, b);
}

Expr make_cell_lit(Expr ex, Expr ey, Expr eb)
{
    struct CellLitCtx *c = malloc(sizeof *c);
    c->ex = ex; c->ey = ey; c->eb = eb;
    return (Expr){ eval_cell_lit, c };
}

struct TypeCheckCtx { Expr a, b; };

static Value eval_typecheck(void *ctx)
{
    struct TypeCheckCtx *c = ctx;
    Value va = c->a.fn(c->a.ctx);
    Value vb = c->b.fn(c->b.ctx);
        if (va.type == VALUE_UNKNOWN || vb.type == VALUE_UNKNOWN)
        return make_bool(1);
    return make_bool(va.type == vb.type);
}

Expr make_typecheck_expr(Expr a, Expr b)
{
    struct TypeCheckCtx *c = malloc(sizeof *c);
    c->a = a; c->b = b;
    return (Expr){ eval_typecheck, c };
}

const char *value_type_name(ValueType type) {
    switch (type) {
        case VALUE_INT:     return "int";
        case VALUE_BOOL:    return "bool";
        case VALUE_CELL:    return "cell";
        case VALUE_ARRAY:   return "array";
        case VALUE_UNKNOWN: return "unknown";
        default:            return "???";
    }
}
