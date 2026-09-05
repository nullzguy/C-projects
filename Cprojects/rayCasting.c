/* COMPILE & RUN WITH:
gcc -O3 -o rCast ./rayCasting.c -lm
./rCast
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define EPSILON 0.001f

#define EMPTY 0
#define WALL 1

// --==& STRUCTS &==--
#pragma region Structs

struct Settings {
    int w, h;
    float fps, wallChance;
}; // 16 bytes

struct Ray{
    float posX, posY,
    dir, castRad;
}; // 16 bytes

struct Cell{
    int type, 
    x, y, idx;
}; // 16 bytes

#pragma endregion

// --==& FUNCTIONS &==--
#pragma region Functions

float dist(float x1, float y1, float x2, float y2) {
    float difX = (x2-x1), difY = (y2-y1);
    return sqrt(difX*difX + difY*difY);
}

void render(struct Settings *s, struct Ray *r, struct Cell *c) {
    for (int y=s->h-1;y>=0;y--) { //bottop to top
        for (int x=0;x<s->w;x++) { //left to right
            int idx = y * s->w + x;
            printf(
                (x==(int)(r->posX+0.5f)&&y==(int)(r->posY+0.5f)) ? "&&" : 
                (c[idx].type==WALL) ? "██" :
                (dist(r->posX,r->posY, x+0.5f,y+0.5f) < r->castRad) ? "▒▒" :
                ".'"
            );
        }
        printf("\n");
    }
}

int rayStep(struct Settings *s, struct Ray *r, struct Cell *c) {
    r->posX += cos(r->dir) * (r->castRad);
    r->posY += sin(r->dir) * (r->castRad);
    r->castRad = 1000000;
    for (int i=0;i<s->w*s->h;i++) {
        if (c[i].type == EMPTY) continue;
        if (r->castRad<=EPSILON) return 1;
        float xDif = c[i].x - r->posX;
        float yDif = c[i].y - r->posY;
        float iDist = (
            (r->posX >= c[i].x && r->posX <= c[i].x+1) ? (yDif < 0) ? -yDif : (yDif > 1 ? yDif - 1 : 0) :
            (r->posY >= c[i].y && r->posY <= c[i].y+1) ? (xDif < 0) ? -xDif : (xDif > 1 ? xDif - 1 : 0) :
            dist(r->posX, r->posY, c[i].x + (xDif > 0 ? 1 : 0), c[i].y + (yDif > 0 ? 1 : 0))
        );
        if (c[i].type == WALL && iDist < r->castRad) r->castRad = iDist;
    }
    return 0;
}

#pragma endregion

// --==& PROGRAM &==--
int main() {
    srand(time(0));

    int rayStatus = 0;

    struct Settings set = {
        80, 80, // width, height
        2, // fps
        1, // wall chance
    };
    struct Ray ray = {
        2, set.h>>1, // posX, posY
        30*(M_PI/180), // direction
        0 // cast radius
    };
    struct Cell cells[set.w*set.h];

    // Initialize cells
    for (int i=0;i<set.w*set.h;i++) {
        int x = i%set.w, y = i/set.w;
        cells[i].type = (
            (x==0||y==0||x==set.w-1||y==set.h-1) ? WALL :
            (set.wallChance > rand()%100) ? WALL : EMPTY
        );
        cells[i].x = x;
        cells[i].y = y;
        cells[i].idx = i;
    }

    while (rayStatus!=1) {
        system("clear"); render(&set, &ray, cells);
        rayStatus = rayStep(&set, &ray, cells);

        struct timespec wait; wait.tv_sec=0; wait.tv_nsec=(1.0f/set.fps)*1e9;
        nanosleep(&wait,NULL);
    }
}