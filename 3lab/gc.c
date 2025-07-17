#include "gc.h"
#include <stdio.h>

static GcNode *head = NULL;

void *gc_alloc(size_t n)
{
    void *p = malloc(n);
    if (!p) { perror("malloc"); exit(1); }
    GcNode *node = malloc(sizeof *node);
    node->ptr = p;
    node->next = head;
    head = node;
    return p;
}

void gc_collect(void)
{
    while (head) {
        GcNode *n = head->next;
        free(head->ptr);
        free(head);
        head = n;
    }
}