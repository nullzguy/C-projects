/* COMPILE & RUN WITH:
gcc -O3 -o path ./pathFinding.c
./path
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define EMPTY 0
#define WALL 1
#define START 2
#define END 3
#define PATH 4

struct Cell {
    int type, open, closed, pIdx, g, h, f, idx;
};

struct Settings {
    int w, h;
    int sps, spr;
    int startIdx, endIdx;
    int lastOpenIdx, lastClosedIdx;
    int wallChance;
};

void render(struct Settings *s, struct Cell *c) {
    char buf[s->h*(s->w*8+2)];
    int pos = 0;
    for (int i=s->h-1;i>=0;i--) { //bottop to top
        for (int j=0;j<s->w;j++) { //left to right
            int idx = i * s->w + j;
            const char *symbol;
            if (c[idx].type == EMPTY) {
                symbol = (c[idx].open == 1) ? "▒▒" : (c[idx].closed == 1) ? "▓▓" : ".'";
            } else if (c[idx].type == WALL) {
                symbol = "██";
            } else if (c[idx].type == START) {
                symbol = "ST";
            } else if (c[idx].type == END) {
                symbol = "EN";
            } else if (c[idx].type == PATH) {
                symbol = "##";
            } else {
                symbol = "??";
            }
            while (*symbol) {
                buf[pos++] = *symbol++;
            }
        }
        buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    printf("%s", buf);
}

int left(struct Settings *s, struct Cell *c) {
    return (c->idx % s->w != 0) ? c->idx-1 : -1;
} int right(struct Settings *s, struct Cell *c) {
    return (c->idx % s->w != s->w-1) ? c->idx+1 : -1;
} int up(struct Settings *s, struct Cell *c) {
    return (c->idx / s->w != 0) ? c->idx - s->w : -1; //flipped
} int down(struct Settings *s, struct Cell *c) {
    return (c->idx / s->w != s->h - 1) ? c->idx + s->w : -1; //flipped
}

void neighborUpdate(struct Settings *s, struct Cell *c, int *open, int lFIdx, int nIdx) {
    if (nIdx != -1 && c[nIdx].type != WALL && c[nIdx].closed != 1) {
        if (c[nIdx].open == 0) {
            c[nIdx].g = c[lFIdx].g+1;
            c[nIdx].pIdx = lFIdx;   // <-- HERE
            open[s->lastOpenIdx] = nIdx;
            s->lastOpenIdx++;
        } 
        else if (c[lFIdx].g+1 < c[nIdx].g) {
            c[nIdx].g = c[lFIdx].g+1;
            c[nIdx].pIdx = lFIdx;   // <-- AND HERE
        }
        c[nIdx].f = c[nIdx].g + c[nIdx].h;
        c[nIdx].open = 1;
    }
}

void aStarStep(struct Settings *s, struct Cell *c, int *open, int *closed) {
    int lFIdx = -1;
    for (int i=0;i<s->w*s->h;i++) {
        if (open[i]==-1 || c[open[i]].type == WALL || c[open[i]].closed == 1) continue;
        lFIdx = (lFIdx == -1) ? c[open[i]].idx : (c[open[i]].f < c[lFIdx].f) ? open[i] : lFIdx;
    }

    if (lFIdx == -1) {
        system("clear");
        printf("\nNO PATH FOUND!\n");
        exit(0);
    }

    if (c[lFIdx].closed != 1) {
        closed[s->lastClosedIdx] = lFIdx;
        s->lastClosedIdx++;
    }
    c[lFIdx].open = 0;
    c[lFIdx].closed = 1;

    int neighbors[4] = {left(s,&c[lFIdx]), right(s,&c[lFIdx]), up(s,&c[lFIdx]), down(s,&c[lFIdx])};
    for (int i=0;i<4;i++) {
        neighborUpdate(s, c, open, lFIdx, neighbors[i]);
    }
}

void reconstructPath(struct Settings *s, struct Cell *c) {
    int pIdx = s->endIdx;
    for (int i=0;i<s->w*s->h;i++) {
        c[pIdx].type = PATH;
        pIdx = c[pIdx].pIdx;
        if (pIdx == s->startIdx) break;
    }
}

int main() {
    srand(time(0));

    struct Settings set = {80, 80, 600, (set.sps != -1) ? (int)((double)set.sps/30.0) : 2e9, 3+76*set.w, 76+3*set.w, 0, 0, 30};
    struct Cell cells[set.w*set.h];

    int open[set.w*set.h];
    int closed[set.w*set.h];

// INITIALIZATION
    for (int i=0;i<set.w*set.h;i++) {
        open[i] = -1;
        cells[i].type = (set.wallChance<rand()%100) ? EMPTY : WALL;
        cells[i].open = 0;
        cells[i].closed = 0;
        cells[i].idx = i;
        cells[i].g = 0;
        cells[i].h = abs((cells[i].idx % set.w)-(cells[set.endIdx].idx % set.w)) + 
        abs((int)((double)cells[i].idx/(double)set.w)-(int)((double)cells[set.endIdx].idx/(double)set.w));
    }
    cells[set.startIdx].type = START, cells[set.startIdx].open = 1, open[0] = set.startIdx, set.lastOpenIdx++;
    cells[set.endIdx].type = END;

    int i=0;
    while (1) {
        printf("\033[H");
        
        clock_t start = clock();

        aStarStep(&set, cells, open, closed);

        clock_t comp = clock();

        if (i%set.spr==0) render(&set, cells);
        
        clock_t rend = clock();

        if (cells[set.endIdx].closed == 1) {
            printf("\033[H");
            reconstructPath(&set, cells);
            render(&set, cells);
            break;
        }

        double compTicks = (double)(comp-start), rendTicks = (double)(rend-start);
        long long compNs = (long long)((compTicks / CLOCKS_PER_SEC) * 1e9),
            rendNs = (long long)((rendTicks / CLOCKS_PER_SEC) * 1e9);
        long long targetNs = (long long)(1e9 / set.sps);
        long long sleepNs = targetNs - compNs;

        if (sleepNs > 0 && set.sps != -1) {
            struct timespec wait;
            wait.tv_sec = 0;
            wait.tv_nsec = sleepNs;
            nanosleep(&wait, NULL);
        }
        if (i%set.spr==0) printf(" CompFT|RendFT (ms): %.3f|%.3f\n", 
            (double)(compNs*1e-6), (double)(rendNs*1e-6));
    i++;}

    return 0;
}