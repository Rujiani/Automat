#include "values.h"
#include <stdio.h>
Value make_int(long long v) {
    return (Value){ .type = VALUE_INT, .i = v };
}

Value make_bool(int b){
    Value r = {0};
    r.type = VALUE_BOOL;
    r.i = !!b;
    return r;
 }

Value make_cell(Cell c) {
    return (Value){ .type = VALUE_CELL, .cell = c };
}

Value make_cell_xyz(long long x,long long y,int busy){
    return make_cell((Cell){x,y,busy});
}

Value make_unknown(void)
{
    Value v = {0};
    v.type  = VALUE_UNKNOWN;
    return v;
}


static inline Value make_empty(void)
{
    Value v = { .type = VALUE_UNKNOWN };
    return v;
}

Value make_array(size_t n)
{
    Value v;
    v.type       = VALUE_ARRAY;
    v.array.size = n;
    v.array.data = NULL;

    v.array.data = (Value *)calloc(n, sizeof(Value));
    if (!v.array.data) {
        perror("calloc (make_array)");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < n; ++i)
        v.array.data[i] = make_unknown();

    return v;
}

void free_value(Value *v){
    if(!v) return;
    if(v->type==VALUE_ARRAY){
        for(size_t i=0;i<v->array.size;++i)
            free_value(&v->array.data[i]);
        free(v->array.data);
        v->array.data=NULL;
    }
    v->type = VALUE_UNKNOWN;
}

void print_value(const Value *v){
    switch(v->type){
        case VALUE_INT:  printf("%lld\n",v->i); break;
        case VALUE_BOOL: printf("%s\n", v->i? "thật":"sai"); break;
        case VALUE_CELL: printf("(%lld, %lld) %s\n",
                                v->cell.x, v->cell.y,
                                v->cell.busy?"busy":"free"); break;
        case VALUE_ARRAY:
            putchar('[');
            for(size_t i=0;i<v->array.size;++i){
                print_value(&v->array.data[i]);
                if(i+1<v->array.size) printf(", ");
            }
            puts("]");
            break;
        default: printf("<unknown>\n");
    }
}

void destroy_value(Value v)
{
    if (v.type == VALUE_ARRAY) {
        for (size_t i = 0; i < v.array.size; ++i)
            destroy_value(v.array.data[i]);   /* рекурсивно  */
        free(v.array.data);
    }
}