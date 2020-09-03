/*-----------------17/01/1998 13.09-----------------
 Write.c by Manowar of Rolling Pixels
--------------------------------------------------*/

#include <string.h>
#include "font4.h"
#include "font20.h"
#include "write.h"

extern unsigned short int *yval;

/*-----------------17/01/1998 13.13-----------------
 Queste sono per un modo a 256 colori
--------------------------------------------------*/
void Write4(BYTE *where, char *mex, int x, int y, unsigned char col)
{
    #define A_LENGHT 4
    #define A_HEIGHT 7
    #define A_INITBLANK 0

    unsigned int i,tx,ty,ty2,off,off2,v,c;

    off=x+yval[y];

    for (i=0; i<strlen(mex); i++)
    {
        v=mex[i]-32;
        for ( ty=0, ty2=0; ty<A_HEIGHT; ty++, ty2+=490 )
            for ( tx=0; tx<A_LENGHT; tx++ )
            {
                /* SLOW
                off2=off+tx+(ty<<8)+(ty<<6)+(i*A_LENGHT)+i;
                c=font4[(v*A_LENGHT)+v+tx+ty2+A_INITBLANK];
                */
                off2=off+tx+yval[ty]+(i<<2)+i;
                c=font4[(v<<2)+v+tx+ty2+A_INITBLANK];
                if (c==1) where[off2]=col;
            }
    }
}

void Write20(BYTE *where, char *mex, int x, int y, unsigned char col)
{
    #define LENGHT 12
    #define HEIGHT 19
    #define INITBLANK 2

    unsigned int i,tx,ty,ty2,off,off2,v,c;

    off=x+yval[y];

    for (i=0; i<strlen(mex); i++)
    {
        v=mex[i]-32;
        for ( ty=0, ty2=0; ty<HEIGHT; ty++, ty2+=1280 )
            for ( tx=0; tx<LENGHT; tx++ )
            {
                off2=off+tx+yval[ty]+(i*LENGHT);
                c=font20[(v*LENGHT)+tx+ty2+INITBLANK];
                if (c==1) where[off2]=col;
            }
    }
}
