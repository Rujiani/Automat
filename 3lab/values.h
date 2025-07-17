#ifndef VALUES_H_
#define VALUES_H_

#include <stdlib.h>
#define MAX_DIMS 8
typedef struct {
    long long x, y;
    int busy;  // 0 or 1
} Cell;

typedef enum {
    VALUE_INT,
    VALUE_BOOL,
    VALUE_CELL,
    VALUE_ARRAY,
    VALUE_UNKNOWN
} ValueType;
typedef struct Value Value;

typedef struct Array {
    unsigned long     size;
    Value     *data;
} Array;

struct Value {
    ValueType type;
    union {
        long long i;
        Cell      cell;
        Array array;
    };
};

Value make_int(long long v);
Value make_bool(int b);

Value make_cell(Cell c);

Value make_cell_xyz(long long x,long long y,int busy);
Value make_unknown(void);

Value make_array(size_t ns);
void free_value(Value *v);
void  print_value(const Value *v);
void destroy_value(Value v);
Value make_unknown(void);

#endif