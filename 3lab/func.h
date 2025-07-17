#ifndef FUNC_H
#define FUNC_H

#include <stddef.h>
#include "types.h"
#include "actions.h"

typedef struct Param {
    ValueType type;
    char     *name;
    struct Param *next;
} Param;

typedef struct {
    Param *head;
    Param *tail;
    size_t count;
} ParamList;

typedef struct Function {
    ValueType   ret_type;
    char       *name;
    ParamList  *params;
    ActionList *body;
} Function;

ParamList *create_param_list(void);
ParamList *single_param(ValueType t, char *n);
ParamList *append_param(ParamList *pl, ValueType t, char *n);

void       add_function(Function *f);
Function  *get_function(const char *name);

Value      run_function(Function *f, Expr *args, size_t argc);

#endif
