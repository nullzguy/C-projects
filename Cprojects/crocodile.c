/* COMPILE & RUN WITH:
gcc -O3 -o croco ./crocodile.c
./croco
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct Crocodile {
    int teeth[11];
    int isClosed;
};

void draw(struct Crocodile *c) {
    printf(
        (c->isClosed != 1) ?
        "      _--'''''-_-'''''--_      \n"
        "     /  ,''''.   ,''''.  \\    \n"
        "    /   :_(=)'   '(=)_;   \\   \n"
        "   |__-__-_--_-_-_--_-__-__|   \n"
        "   \\  uUU U  U U U  U UUu  /  \n"
        "    \\ u    ,---_---.    u /   \n"
        "     ;    /    U    \\    ;    \n"
        "     | %c  :;;;;;;;;;: %c%c |  \n"
        "     | %c  ':::::::::' %c%c |  \n"
        "     |'- %c %c ''''' %c %c -'| \n"
        "      \\ ''--.%c_%c_%c.--'' /  \n"
        "       ''-___________-''       \n"
        :
        "      _--'''''-_-'''''--_      \n"
        "     /  ,''--.   ,--''.  \\    \n"
        "    /   :_<=-'   '-=>_:   \\   \n"
        "   |__-__-:::::::::::-__-__|   \n"
        "   |  uUU U  U U U  U UUu  |   \n"
        "    \\_  ''^^.T_T_T.^^''  _/   \n"
        "      '''-___________-'''      \n"
        ,

        (c->teeth[0]==1) ? '1' : 'm',
        (c->teeth[10]==1) ? '1' : 'n', (c->teeth[10]==1) ? '1' : 'n',
        (c->teeth[1]==1) ? '2' : 'm',
        (c->teeth[9]==1) ? '1' : 'n', (c->teeth[9]==1) ? '0' : 'n',
        (c->teeth[2]==1) ? '3' : 'n', (c->teeth[3]==1) ? '4' : 'n',
        (c->teeth[7]==1) ? '8' : 'n', (c->teeth[8]==1) ? '9' : 'n',
        (c->teeth[4]==1) ? '5' : 'n', (c->teeth[5]==1) ? '6' : 'n', (c->teeth[6]==1) ? '7' : 'n'
    );
}

int main() {
    srand(time(0));
    struct Crocodile croco = {{1,1,1,1,1,1,1,1,1,1,1}, 0};
    int inputTooth, turn = 0, error = 0, badTooth = rand()%11;
    char* errors[] = {"Failed to parse input!","Tooth already pressed!","Out of tooth boundaries!"};

    while (1) {
        system("clear");
        draw(&croco);
        
        if (croco.isClosed) {
            printf("\nPlayer %d got biten!", turn%2+1);
            break;
        }

        if (error != 0) {
            printf("\n Error %d: %s", error, errors[error-1]);
        }
        error = 0;

        printf("\n Player %d | Choose tooth (1-11):", turn % 2 + 1);

        int a = scanf("%d", &inputTooth);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}

        if (a!=1) {
            error = 1; continue;
        }

        if (inputTooth > 0 && inputTooth < 12) {
            if (croco.teeth[inputTooth-1] == 0) {
                error = 2; continue;
            } else {
                croco.teeth[inputTooth-1] = 0;
                if (inputTooth == badTooth+1) {
                    croco.isClosed = 1;continue;
                }
            }
        } else {
            error = 3; continue;
        }

        turn++;
    }
}