#ifndef GC_H_
#define GC_H_
#include <stdlib.h>
typedef struct GcNode { void *ptr; struct GcNode *next; } GcNode;
void *gc_alloc(size_t);
void  gc_collect(void);

#endif