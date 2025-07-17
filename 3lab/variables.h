#ifndef VARIABLES_H
#define VARIABLES_H
#include "values.h"
#include "func.h"

typedef struct {
    char  *name;
    Value  val;
    int    is_const;
} Var;

int var_exists(const char* name);
Value get_var(const char *name);
void      set_var(const char *name, Value value);
void      add_var(const char *name);
void add_var_with_type(const char *name, ValueType type);
void          add_function(Function *f);
Function     *get_function(const char *name);
void frame_push(void);
void frame_pop(void);
size_t vars_size(void);
void   vars_pop_to(size_t mark);
void ensure_type(Value v, ValueType expected, const char *name);


#endif