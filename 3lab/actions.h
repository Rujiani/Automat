#ifndef ACTIONS_H
#define ACTIONS_H
#include "types.h"

typedef struct ActionNode {
    Action              a;
    struct ActionNode  *next;
} ActionNode;

typedef struct {
    ActionNode *head, *tail;
} ActionList;
extern int sequence_break;
extern int seq_depth;

void action_list_init(ActionList*);
void action_list_add (ActionList*, Action a);
void action_list_run (const ActionList*);
// void action_list_free(ActionList*);
Value make_array_runtime(size_t n);

typedef struct { int dx, dy; } MoveCtx;
void   robot_move(void *ctx);
Action  make_move_action(int dx, int dy);

typedef struct { int dx, dy; } SenseCtx;
void   robot_sense(void *ctx);
Action  make_sense_action(int dx, int dy);
Value  sequence_flush_overview(void);
Action make_stop_if_obstacle(void);
void action_list_free(ActionList *list);
#endif
