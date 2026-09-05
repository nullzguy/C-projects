/* COMPILE & RUN WITH:
gcc -O3 -o ball ./ballPhysics.c -lm
./ball
*/

#include <stdio.h>
#include <math.h>
#include <time.h>

#define EPSILON 2e-2f

struct Ball {
    float posX, posY, rad;
    float velX, velY;
    float bnc;
};

struct Settings {
    int w, h;
    float fps;
    float g;
    float airR, gndR;
};

float dist(float x1, float y1, float x2, float y2) {
    float difX = (x2-x1), difY = (y2-y1);
    return sqrt(difX*difX + difY*difY);
}

void render(struct Settings *s, struct Ball *b) {
    for (int i=s->h-1;i>=0;i--) { //bottop to top
        for (int j=0;j<s->w;j++) { //left to right
            float d = dist(b->posX, b->posY, j+0.5f, i+0.5f);
            printf(
                (d < b->rad-0.125) ?
                (d < b->rad-0.25) ?
                (d < b->rad-0.375) ?
                (d < b->rad-0.5) ?
                "██" : "▓▓" : "▒▒" : "░░" : ".'"
                // works like anti-aliasing
                // works better with bigger ball radiuses
            );
        }
        printf("\n");
    }
}

void physicsStep(struct Settings *s, struct Ball *b) {
    b->posX += b->velX/s->fps; b->posY += b->velY/s->fps; //apply movement

    if (fabsf(b->velX) < EPSILON) b->velX = 0; //kill little movement for X
    if (fabsf(b->velY) < EPSILON) b->velY = 0; //for Y too

    b->velX *= powf(s->airR, 1/s->fps); //apply air resistance
    if (b->posY-b->rad < 0.01) b->velX *= powf(s->gndR, 1/s->fps); //apply ground resistance
    
    b->velY -= s->g/s->fps; //apply gravity

    if (b->posY-b->rad <= 0.0) { //ground hit check
        b->posY = b->rad;
        b->velY *= -1*b->bnc;
    } else if (b->posY+b->rad >= s->h) { //ceiling hit check
        b->posY = s->h-b->rad;
        b->velY *= -1*b->bnc;
    }
    if (b->posX-b->rad <= 0.0) { //left wall hit check
        b->posX = b->rad;
        b->velX *= -1*b->bnc;
    } else if (b->posX+b->rad >= s->w) { //right wall hit check
        b->posX = s->w-b->rad;
        b->velX *= -1*b->bnc;
    }
}

int main() {
    struct Settings set = {20, 20, 60.0, 9.8, 0.9, 0.8};
    struct Ball ball = {10, 5, 4, 30, 10, 0.5};

    while (1) {
        printf("\033[H");
        
        clock_t start = clock();

        physicsStep(&set, &ball);

        clock_t end = clock();
        
        render(&set, &ball);

        double compTicks = (double)(end-start);
        long long compNs = (long long)((compTicks / CLOCKS_PER_SEC) * 1e9);
        long long targetNs = (long long)(1e9 / set.fps);
        long long sleepNs = targetNs - compNs;

        if (sleepNs > 0) {
            struct timespec wait;
            wait.tv_sec = 0;
            wait.tv_nsec = sleepNs;
            nanosleep(&wait, NULL);
        }
        printf(" FPS|TargetFPS: %.1f|%.1f\n", (double)(1e9 / sleepNs), set.fps);
        printf(" CompFT|FT|TargetFT (ms): %.3f|%.3f|%.3f\n", 
            (double)(compNs*1e-6), (double)(sleepNs*1e-6), (double)(targetNs*1e-6));
    }
}
