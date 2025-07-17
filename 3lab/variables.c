#include "variables.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "values.h"

#include <stdlib.h>
#include "variables.h"

typedef struct {
    char      *name;
    Value      val;
    ValueType  type;
} VarSlot;

static VarSlot *stack   = NULL;
static size_t   cap     = 0;
static size_t   top     = 0;

static void ensure_cap(size_t need)
{
    if (need <= cap) return;
    cap = cap ? cap * 2 : 8;
    while (cap < need) cap *= 2;
    stack = realloc(stack, cap * sizeof *stack);
}

size_t vars_size(void)
{
    return top;
}

void vars_pop_to(size_t mark)
{
    if (mark > top) return;
    top = mark;
}


#define VAR_CAP 512

#define FRAME_DEPTH 32

typedef struct {
    Var vars[VAR_CAP];
    size_t count;
} VarFrame;

static VarFrame frame_stack[FRAME_DEPTH];
static int frame_top = -1;


static Var table[VAR_CAP];
static size_t count = 0;

static int find_idx(const char *n){
    for(size_t i=0;i<count;++i) if(strcmp(table[i].name,n)==0) return (int)i;
    return -1;
}

void add_var(const char *name)
{
    if (count >= VAR_CAP) { fputs("symtab overflow\n", stderr); exit(1); }
    if (find_idx(name) != -1) {
        fprintf(stderr, "variable redeclaration: %s\n", name);
        exit(1);
    }
    table[count].name = strdup(name);

    table[count].val  = make_unknown();
    ++count;
}


void set_var(const char *name, Value v)
{
    int idx = find_idx(name);
    if (idx == -1) { fprintf(stderr,"undeclared variable: %s\n",name); exit(1); }

    if (table[idx].val.type == VALUE_ARRAY) {
        fprintf(stderr, "cannot reassign array variable: %s\n", name);
        exit(1);
    }
    if (table[idx].val.type == VALUE_UNKNOWN) {
        table[idx].val = v;
        return;
    }
    if (table[idx].val.type != v.type) {
        fprintf(stderr, "type mismatch in assignment to %s\n", name);
        exit(1);
    }
    table[idx].val = v;
}

Value get_var(const char *name){
    int idx = find_idx(name);
    if(idx==-1){ fprintf(stderr,"undeclared variable: %s\n",name); exit(1);}
    return table[idx].val;
}

int var_exists(const char *name){ return find_idx(name)!=-1; }

void add_var_with_type(const char *name, ValueType type)
{
    if (count >= VAR_CAP) { fputs("symtab overflow\n", stderr); exit(1); }
    if (find_idx(name) != -1) {
        fprintf(stderr, "variable redeclaration: %s\n", name);
        exit(1);
    }

    Value v;
    v.type = type;
    switch (type) {
        case VALUE_INT:  v = make_int(0);  break;
        case VALUE_BOOL: v = make_bool(0); break;
        case VALUE_CELL: v = make_cell_xyz(0,0,0); break;
        case VALUE_ARRAY:  v.array.data = NULL; v.array.size = 0; break;
        default:           v = make_unknown(); break;
    }

    table[count].name = strdup(name);
    table[count].val  = v;
    ++count;
}

void frame_push(void) {
    if (frame_top + 1 >= FRAME_DEPTH) {
        fprintf(stderr, "Too many nested frames\n");
        exit(1);
    }
    ++frame_top;
    frame_stack[frame_top].count = count;
    memcpy(frame_stack[frame_top].vars, table, sizeof(Var) * count);
    count = 0;
}

void frame_pop(void) {
    if (frame_top < 0) {
        fprintf(stderr, "No frame to pop\n");
        exit(1);
    }
    memcpy(table, frame_stack[frame_top].vars, sizeof(Var) * frame_stack[frame_top].count);
    count = frame_stack[frame_top].count;
    --frame_top;
}

void ensure_type(Value v, ValueType expected, const char *name) {
    if (v.type != expected) {
        fprintf(stderr, "type mismatch for '%s': expected %d, got %d\n",
                name, expected, v.type);
        exit(1);
    }
}
