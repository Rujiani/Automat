
#include "actions.h"
#include "robot.h"
#include "maze.h"
#include "variables.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>

int seq_depth = 0;
int sequence_break = 0;
void action_list_init(ActionList *list)
{
    list->head = list->tail = NULL;
}

void action_list_add(ActionList *list, Action a)
{
    ActionNode *n = malloc(sizeof *n);
    n->a   = a;
    n->next = NULL;

    if (!list->head)
        list->head = list->tail = n;
    else {
        list->tail->next = n;
        list->tail       = n;
    }
}

void action_list_run(const ActionList *list)
{
    for (ActionNode *n = list->head; n; n = n->next)
        if (n->a.fn)
            n->a.fn(n->a.ctx);
}


Value make_array_runtime(size_t n) { return make_array(n); }


Action make_move_action(int dx, int dy)
{
    MoveCtx *c = malloc(sizeof *c);
    c->dx = dx; c->dy = dy;
    return (Action){ .fn = robot_move, .ctx = c };
}

void robot_move(void *vctx)
{
    MoveCtx *c = vctx;
    robot_move_xy(c->dx, c->dy);
}


extern int sequence_break;

static Value overview_matrix = { .type = VALUE_UNKNOWN };


Action make_sense_action(int dx, int dy)
{
    SenseCtx *c = malloc(sizeof *c);
    c->dx = dx; c->dy = dy;
    return (Action){ .fn = robot_sense, .ctx = c };
}

void robot_sense(void *vctx)
{
    SenseCtx *c = vctx;

    int dist = 0;
    int x = robo.x, y = robo.y;
    while (!maze_is_obstacle(x + c->dx, y + c->dy)) {
        x += c->dx;  y += c->dy;  ++dist;
    }

    if (!var_exists("_sense_result"))
        add_var_with_type("_sense_result", VALUE_INT);
    set_var("_sense_result", make_int(dist));

    if (seq_depth > 0) {
        Value row = make_array(dist);
        int cx = robo.x, cy = robo.y;
        for (int i = 0; i < dist; ++i) {
            cx += c->dx;  cy += c->dy;
            row.array.data[i] = make_cell_xyz(cx, cy,
                                   maze_is_obstacle(cx, cy));
        }

        if (overview_matrix.type == VALUE_UNKNOWN)
            overview_matrix = make_array_runtime(0);

        size_t oldh = overview_matrix.array.size;
        overview_matrix.array.size  = oldh + 1;
        overview_matrix.array.data  = realloc(
            overview_matrix.array.data, (oldh + 1) * sizeof(Value));
        overview_matrix.array.data[oldh] = row;
    }

    if (dist == 0)
        sequence_break = 1;
}

Value sequence_flush_overview(void)
{
    Value res = (overview_matrix.type == VALUE_UNKNOWN)
                ? make_unknown()
                : overview_matrix;

    overview_matrix.type = VALUE_UNKNOWN;
    overview_matrix.array.size = 0;
    overview_matrix.array.data = NULL;
    return res;
}


static void stop_if_obstacle(void *unused)
{
    (void)unused;

    Value v = get_var("_sense_result");

    if (v.type != VALUE_INT) {
        fprintf(stderr, "_sense_result: waiting int\n");
        exit(1);
    }

    if (v.i == 0)
        sequence_break = 1;
}

Action make_stop_if_obstacle(void)
{
    Action a;
    a.fn  = stop_if_obstacle;
    a.ctx = NULL;
    return a;
}


static int ctx_needs_free(Action a)
{
    return  a.fn == robot_move      ||
            a.fn == robot_sense     ||
            a.fn == stop_if_obstacle;
}

void action_list_free(ActionList *list)
{
    for (ActionNode *n = list->head; n; ) {
        ActionNode *next = n->next;

        if (ctx_needs_free(n->a) && n->a.ctx)
            free(n->a.ctx);

        free(n);
        n = next;
    }
    list->head = list->tail = NULL;
}
