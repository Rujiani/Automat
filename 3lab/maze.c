#include "maze.h"
#include "variables.h"      /* add_var_with_type, set_var */
#include <stdio.h>
#include <stdlib.h>
#include "robot.h"

int W, H;
static int **grid;
static int exitX, exitY;

int maze_is_obstacle(int x, int y)
{
    if (x < 0 || y < 0 || y >= H || x >= W) return 1;
    return grid[y][x];
}

void read_maze(const char *fname)
{
    FILE *f = fopen(fname,"r");
    if (!f){ perror(fname); exit(1); }

    if (fscanf(f,"%d %d",&W,&H)!=2){fprintf(stderr,"bad size\n");exit(1);}

    grid = malloc(H*sizeof *grid);
    for (int y=0;y<H;++y){
        grid[y]=malloc(W*sizeof *grid[y]);
        for (int x=0;x<W;++x){
            int ch=fgetc(f);
            while (ch=='\n' || ch=='\r') ch=fgetc(f);
            if (ch!='0' && ch!='1'){fprintf(stderr,"bad map\n");exit(1);}
            grid[y][x]= (ch=='1');
        }
    }

    if (fscanf(f,"%Ld %Ld",&robo.x,&robo.y)!=2) {fprintf(stderr,"bad start\n"); exit(1);}
    if (fscanf(f,"%d %d",&exitX,&exitY)!=2)   {fprintf(stderr,"bad exit\n");  exit(1);}
    fclose(f);

    add_var_with_type("exit_x", VALUE_INT);
    add_var_with_type("exit_y", VALUE_INT);
    set_var("exit_x", make_int(exitX));
    set_var("exit_y", make_int(exitY));

}

int maze_is_exit(int x, int y) {
    return x == exitX && y == exitY;
}


void maze_free(void)
{
    if (grid) {
        for (int y = 0; y < H; ++y)
            free(grid[y]);
        free(grid);
        grid = NULL;
    }

    W = H = 0;
}