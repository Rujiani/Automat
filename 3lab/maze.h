#ifndef MAZE_H
#define MAZE_H
#include <stdbool.h>

void read_maze(const char *fname);
int  maze_is_obstacle(int x,int y);
int maze_is_exit(int x, int y);
void maze_free(void);
#endif
