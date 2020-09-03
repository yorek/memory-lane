/*-----------------17/01/1998 13.09-----------------
 Write.h by Grizly of Rolling Pixels
--------------------------------------------------*/
#ifndef _RPS_WRITE
#define _RPS_WRITE


typedef unsigned char BYTE;

void Write4(BYTE *where, char *mex, int x, int y, unsigned char col);
void Write20(BYTE *where, char *mex, int x, int y, unsigned char col);

#endif