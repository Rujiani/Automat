#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "func.h"
#include "variables.h"
#include "gc.h"

extern int   seq_depth;
extern Value last_overview;

typedef struct FuncEntry {
    char *name;
    Function *def;
    struct FuncEntry *next;
} FuncEntry;

static FuncEntry *func_table = NULL;


void add_function(Function *def)
{
    FuncEntry *e = malloc(sizeof *e);
    e->name = strdup(def->name);
    e->def  = def;
    e->next = func_table;
    func_table = e;
}

Function *get_function(const char *name)
{
    for (FuncEntry *e = func_table; e; e = e->next)
        if (strcmp(e->name, name) == 0)
            return e->def;
    return NULL;
}


ParamList *create_param_list(void)
{
    ParamList *pl = gc_alloc(sizeof *pl);
    pl->head = pl->tail = NULL;
    pl->count = 0;
    return pl;
}

ParamList *single_param(ValueType t, char *n)
{
    ParamList *pl = create_param_list();
    return append_param(pl, t, n);
}

ParamList *append_param(ParamList *pl, ValueType t, char *n)
{
    Param *p = gc_alloc(sizeof *p);
    p->type = t;
    p->name = n;
    p->next = NULL;

    if (pl->tail)
        pl->tail->next = p;
    else
        pl->head = p;

    pl->tail = p;
    ++pl->count;
    return pl;
}


Value run_function(Function *f, Expr *argv, size_t argc)
{
    if (argc != f->params->count) {
        fprintf(stderr, "wrong arg count\n");
        exit(1);
    }

    size_t mark = vars_size();

    Param *p = f->params->head;
    for (size_t i = 0; i < argc; ++i, p = p->next) {
        Value v = argv[i].fn(argv[i].ctx);
        ensure_type(v, p->type, p->name);
        add_var_with_type(p->name, p->type);
        set_var(p->name, v);
    }

    ++seq_depth;                 /* печать внутри функции запрещаем   */
    last_overview = make_unknown();

    action_list_run(f->body);

    --seq_depth;

    Value ret = last_overview;
    vars_pop_to(mark);
    return ret;
}
