#ifndef ROBOT_H
#define ROBOT_H

struct Robot {
    long long x, y;
};

extern struct Robot robo;

void robot_move_xy(int dx, int dy);
int  robot_sense_xy(int dx, int dy);
void robot_init(int x, int y);
#endif
