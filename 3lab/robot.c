#include "robot.h"
#include "maze.h"
#include <stdio.h>
#include <stdlib.h>


long long step_counter = 0;
struct Robot robo;

void robot_init(int x, int y) { robo.x = x; robo.y = y; }



void robot_move_xy(int dx, int dy)
{
    int ox = robo.x, oy = robo.y;
    int nx = ox + dx, ny = oy + dy;

    if (maze_is_obstacle(nx, ny)) {
        printf("crash (%d,%d)\n", nx, ny);
        fflush(stdout);
        exit(1);
    }

    robo.x = nx;
    robo.y = ny;
    printf("[step %lld] (%d,%d) -> (%d,%d)\n",
           ++step_counter, ox, oy, nx, ny);
    fflush(stdout);

    if (maze_is_exit(nx, ny)) {
        printf("finish\n");
        fflush(stdout);
        exit(0);
    }
}

int robot_sense_xy(int dx, int dy) {
    int x = robo.x, y = robo.y, d = 0;
    while (!maze_is_obstacle(x + dx, y + dy)) { x+=dx; y+=dy; ++d; }
    return d;
}