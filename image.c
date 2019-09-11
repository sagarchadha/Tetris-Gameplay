//#include "image.s"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
 
//Definitions
#define pbEdge ((volatile long *) 0xFF20005C)
extern short MYIMAGE [240][320];
extern short ENDGAMEIMAGE [240][320];

int main (){
    volatile short * pixelbuf = 0xc8000000;
    int i, j;
    for (i=0; i<240; i++)
        for (j=0; j<320; j++)
            *(pixelbuf + (j<<0) + (i<<9)) = MYIMAGE[i][j];
     while (*pbEdge != 1) {
            if (*pbEdge != 0 && *pbEdge != 1)
                *pbEdge = 15;
     }
     *pbEdge = 15;
    int x, y;
    for (x=0; x<240; x++)
        for (y=0; y<320; y++)
            *(pixelbuf + (y<<0) + (x<<9)) = ENDGAMEIMAGE[x][y];
    return 0;
}
