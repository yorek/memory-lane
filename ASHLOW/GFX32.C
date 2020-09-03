// File: RpsDemo.c
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>
#include <dos.h>
#include <math.h>
#include <i86.h>
#include "gfx32.h"
#include "font8.h"

//
// BUFFER VGA
//
BYTE *vga;

//
// Buffer per disegnare un poligono pieno
//
int xlist[200][2];

//
// Buffer che contiene i valori del seno
//
int sine[kangle360];

//
// Buffer Palette
//
RGB curpal[256];     // Qui si trova la palette corrente
RGB destpal[256];    // Qui si trova la palette di destinazione

//
// Variabili per la rotazione.
// Par ruotare in punto:
// . In x,y,z si mettono le coordinate del punto.
// . Si chiama la routine UpdateRotation(...) in cui si passa
//   il valore dell'angolo di rotazione per l'asse x,y e z
//   Automaticamente vengono aggiornati i valori xangle, yangle e zangle
// . Si chiama la routine RotatePoints
// . In x,y,z si trovano i valori routati.
// . Per trasformarli da 3d a 2d con prospettiva:
//   ((x<<8)/(z-zoff))+centro x
//   ((y<<8)/(z-zoff))+centro y
int x=0,y=0,z=0;
int xt=0,yt=0,zt=0;
int zoff=-512;
int xsin=0,xcos=0;
int ysin=0,ycos=0;
int zsin=0,zcos=0;
BYTE xangle=0,yangle=0,zangle=0;

// Dimensioni dello schermo nella modalità settata
extern int scrx, scry;

/*-------------------------------------------------------------------------*/
/* DISEGNA UNA LINEA NUL BUFFER PASSATO                                    */
/*-------------------------------------------------------------------------*/
void Bline(int x1, int y1, int x2, int y2, BYTE color, BYTE *where)
{
   // Per il tracciamento della linea
   int i,deltax,deltay,numpix,d,dinc1,dinc2,xb,xinc1,xinc2,yb,yinc1,yinc2;
   int off;

	 deltax=x2-x1;
   if (deltax<0) deltax=-deltax;
   deltay=y2-y1;
   if (deltay<0) deltay=-deltay;

   if (deltax >= deltay) {
      numpix=deltax;
      numpix++;
      d=(deltay<<1)-deltax;
      dinc1=deltay<<1;
      dinc2=(deltay-deltax)<<1;
      xinc1=1;
      xinc2=1;
      yinc1=0;
      yinc2=1;
   } else {
      numpix=deltay;
      numpix++;
      d=(deltax<<1)-deltay;
      dinc1=deltax<<1;
      dinc2=(deltax-deltay)<<1;
      xinc1=0;
      xinc2=1;
      yinc1=1;
      yinc2=1;
   }

   if (x1>x2) {
      xinc1 = -xinc1;
      xinc2 = -xinc2;
   }

   if (y1>y2) {
      yinc1 = -yinc1;
      yinc2 = -yinc2;
   }
   xb=x1;
   yb=y1;

    for (i=0;i<numpix;i++) {
        if ( ( xb>0 ) && ( xb<scrx-1) && ( yb>0 ) && ( yb<scry-1) )
        {
            //off=xb+(yb<<8)+(yb<<6);   // Sorry it's slower but .....
            off = xb + scrx*yb;
            where[off]=color;
        }

        if (d<0) {
            d+=dinc1;
            xb+=xinc1;
            yb+=yinc1;
        } else {
            d+=dinc2;
            xb+=xinc2;
            yb+=yinc2;
        }
   }
}

/*-------------------------------------------------------------------------*/
/* DISEGNA UNA POLIGONO DI 4 LATI IN WHERE DEL COLORE SCELTO               */
/*-------------------------------------------------------------------------*/
void Draw_Polygon( int  x1,  int  y1,  int  x2,  int  y2,  int  x3,  int  y3,
                   int  x4,  int  y4, BYTE color, BYTE *where)
{
   int miny,maxy,loop;

   for ( loop=0; loop<200; loop++ )
   {
        xlist[loop][0]=320;
        xlist[loop][1]=0;
   }

   miny=y1;
   maxy=y1;
   if (y2<miny) miny=y2;
   if (y3<miny) miny=y3;
   if (y4<miny) miny=y4;
   if (y2>maxy) maxy=y2;
   if (y3>maxy) maxy=y3;
   if (y4>maxy) maxy=y4;

   if (miny<0) miny=0;
   if (maxy>199) maxy=199;
   if (miny>199) return;
   if (maxy<0) return;

   ScanEdge(x1,y1,x2,y2);
   ScanEdge(x2,y2,x3,y3);
   ScanEdge(x3,y3,x4,y4);
   ScanEdge(x4,y4,x1,y1);

   for (loop=miny; loop<maxy; loop++)
      Hline(xlist[loop][0],xlist[loop][1],loop,color,where);
}

void ScanEdge(int x1,  int y1,  int x2,  int y2)
{
   long x,xstep;
   int loop,temp;

   if (y1==y2) return;
   if (y1>y2) {
      temp=y1;
      y1=y2;
      y2=temp;

      temp=x1;
      x1=x2;
      x2=temp;
   }

   xstep=(x2-x1);
   xstep<<=8;
   xstep/=(y2-y1);

   x=x1;
   x<<=8;

   for (loop=y1; loop<y2; loop++)
   {
      if ((x>>8) < xlist[loop][0]) xlist[loop][0]=(int)(x>>8);
      if ((x>>8) > xlist[loop][1]) xlist[loop][1]=(int)(x>>8);
      x+=xstep;
   }
}

/*-------------------------------------------------------------------------*/
/* CAMBIA IL COLORE NUMCOL CON I VOLORI R,G,B PASSATI                      */
/*-------------------------------------------------------------------------*/
void SetColor(BYTE numcol, BYTE r, BYTE g, BYTE b)
{
   outp(0x3c8,numcol); // Numero colore
   outp(0x3c9,r);      // Valore di R
   outp(0x3c9,g);      // Valore di G
   outp(0x3c9,b);      // Valore di B
}

/*-------------------------------------------------------------------------*/
/* MEMORIZZA LA PALETTE CORRENTE IN CURPAL[]                               */
/*-------------------------------------------------------------------------*/
void GetPalette(void)
{
   WORD i;

   // Inizia dal colore 0
   outp(0x3c7,0x0);
   for (i=0;i<256;i++) {
      curpal[i].r=(BYTE)inp(0x3c9);
      curpal[i].g=(BYTE)inp(0x3c9);
      curpal[i].b=(BYTE)inp(0x3c9);
   }
}

/*-------------------------------------------------------------------------*/
/* EFFETTUA UN FADE IN COMPLETO                                            */
/*-------------------------------------------------------------------------*/
void Fade_In(void)
{
   BYTE flag=1;

   do {
      flag=update_fade_in();
      WaitRet();
   } while (flag==1);
}

/*-------------------------------------------------------------------------*/
/* EFFETTUA UNO STEP DI FADE IN. sE E' COMLETO RITORNA 0                   */
/*-------------------------------------------------------------------------*/
int update_fade_in(void)
{
   int i;
   unsigned char flag=0;

   for(i=0;i<256;i++) {

         if (curpal[i].r<destpal[i].r)
            curpal[i].r++;
         if (curpal[i].r>destpal[i].r)
            curpal[i].r--;

         if (curpal[i].g<destpal[i].g)
            curpal[i].g++;
         if (curpal[i].g>destpal[i].g)
            curpal[i].g--;

         if (curpal[i].b<destpal[i].b)
            curpal[i].b++;
         if (curpal[i].b>destpal[i].b)
            curpal[i].b--;

         SetColor(i,curpal[i].r,curpal[i].g,curpal[i].b);
   }
   flag=0;
   for (i=0;i<256;i++) {
      if (curpal[i].r != destpal[i].r) {
         flag=1;
         break;
      }
      if (curpal[i].g != destpal[i].g) {
         flag=1;
         break;
      }
      if (curpal[i].b != destpal[i].b) {
         flag=1;
         break;
      }
   }
   return flag;
}

/*-------------------------------------------------------------------------*/
/* EFFETTUA UNO FADE OUT AL NERO O AL BIANCO.                              */
/*-------------------------------------------------------------------------*/
void Fade_To_Black(BYTE from, BYTE to)
{
   int flag=0;

   do {
      // se flag=0 allora il fade Š finito
      flag=update_fade_to_black(from,to);
      WaitRet();
   } while(flag==1);

}

int update_fade_to_black(BYTE from, BYTE to)
{
   int i;
   int flag;

   for (i=from; i<to+1; i++) {
      if (curpal[i].r>0) curpal[i].r--;
      if (curpal[i].g>0) curpal[i].g--;
      if (curpal[i].b>0) curpal[i].b--;

      SetColor(i,curpal[i].r,curpal[i].g,curpal[i].b);
   }

   flag=0;
   for (i=from; i<to+1; i++) {
      if (curpal[i].r != 0) {
         flag=1;
         break;
      }
      if (curpal[i].g != 0) {
         flag=1;
         break;
      }
      if (curpal[i].b != 0) {
         flag=1;
         break;
      }
   }
   return flag;
}

void Fade_to_white(BYTE from, BYTE to)
{
   unsigned char flag=0;

   do {
      // se flag=0 allora il fade Š finito
      flag=update_fade_to_white(from,to);
      WaitRet();
   } while(flag==1);

}

int update_fade_to_white(BYTE from, BYTE to)
{
   int i;
   unsigned char flag;

   for (i=from; i<to+1; i++) {
      if (curpal[i].r<63) curpal[i].r++;
      if (curpal[i].g<63) curpal[i].g++;
      if (curpal[i].b<63) curpal[i].b++;

      SetColor(i,curpal[i].r,curpal[i].g,curpal[i].b);
   }

   flag=0;
   for (i=from; i<to+1; i++) {
      if (curpal[i].r != 63) {
         flag=1;
         break;
      }
      if (curpal[i].g != 63) {
         flag=1;
         break;
      }
      if (curpal[i].b != 63) {
         flag=1;
         break;
      }
   }
   return flag;
}

/*-------------------------------------------------------------------------*/
/* STAMPA UN LINEA DI TESTO                                                */
/*-------------------------------------------------------------------------*/
void Stampa(BYTE *where, char *mex, int y, BYTE ct, BYTE mode)
{
    WORD i,it,v,vt,r,c,x,xt,yt;
    BYTE col;

    x=0;
    x=(320-((strlen(mex)<<3)));
    x>>=1;

    for (i=0; i<strlen(mex); i++) {

        switch (mex[i]) {
            case 32: v=26; break;   // " "
            case 46: v=27; break;   // "."
            case 33: v=28; break;   // "!"
            case 63: v=29; break;   // "?"
            case 40: v=30; break;   // "("
            case 41: v=31; break;   // ")"
            case 45: v=32; break;   // "-"
            case 48: v=33; break;   // "0"
            case 49: v=34; break;   // "1"
            case 50: v=35; break;   // "2"
            case 51: v=36; break;   // "3"
            case 52: v=37; break;   // "4"
            case 53: v=38; break;   // "5"
            case 54: v=39; break;   // "6"
            case 55: v=40; break;   // "7"
            case 56: v=41; break;   // "8"
            case 57: v=42; break;   // "9"
            case 39: v=43; break;   // "'"
            default: v=mex[i]-65;
        }

        it=(i<<3);
        vt=(v<<6);

        for (c=0; c<8; c++)
            for(r=0; r<8; r++) {
                xt=x+it+r;
                yt=y+c;
                if (fnt8[vt+(c<<3)+r]!=0)
                    where[xt+(yt<<8)+(yt<<6)]=ct;

            }

        if ( mode==SMOOTH )
        {
            for (c=0; c<8; c++)
                for(r=0; r<8; r++) {
                    xt=x+it+r;
                    yt=y+c;
                    col=where[xt+(yt<<8)+(yt<<6)];
                    col=SmoothPix(xt,yt,col,where);
                    where[xt+(yt<<8)+(yt<<6)]=col;
                }
        }

    }
}

/*-------------------------------------------------------------------------*/
/* FUNZIONE SmoothPix                                                      */
/*-------------------------------------------------------------------------*/
BYTE SmoothPix(WORD x, WORD y, BYTE c, BYTE *where)
{
    WORD r;

    r=c;
    r<<=2;

    r+=where[x+(y<<8)+(y<<6)-320];
    r+=where[x+(y<<8)+(y<<6)+320];
    r+=where[x+(y<<8)+(y<<6)+1];
    r+=where[x+(y<<8)+(y<<6)-1];

    r>>=3;
    return (BYTE)r;
}

/*-------------------------------------------------------------------------*/
/* GENERA LA TABELLA VIRTUALE PER IL SENO                                  */
/*-------------------------------------------------------------------------*/
void CalculateSine()
{
    int  i;
    for (i=0; i<kangle360; i++)
        sine[i]=(int)((sin(i*kpi/kangle180)) * 256);
}

/*-------------------------------------------------------------------------*/
/* GENERA UN NUMERO RANDOM                                                 */
/*-------------------------------------------------------------------------*/
int random (int maxvalue) {
    return (rand()%maxvalue);
}


/*-------------------------------------------------------------------------*/
/* RUOTA I PUNTI X,Y,Z                                                     */
/*-------------------------------------------------------------------------*/
void RotatePoint(void)
{
    yt = (( y * xcos - z * xsin ) >> 8);
    zt = (( y * xsin + z * xcos ) >> 8);
    y=yt;
    z=zt;

    xt = (( x * ycos - z * ysin ) >> 8);
    zt = (( x * ysin + z * ycos ) >> 8);
    x=xt;
    z=zt;

    xt = (( x * zcos - y * zsin) >> 8);
    yt = (( x * zsin + y * zcos) >> 8);
    x=xt;
    y=yt;
}

/*-------------------------------------------------------------------------*/
/* AGGIORNA GLI ANGOLI DI ROTAZIONE                                        */
/*-------------------------------------------------------------------------*/
void UpdateRotation(BYTE deltax, BYTE deltay, BYTE deltaz)
{
    xangle+=deltax;
    yangle+=deltay;
    zangle+=deltaz;

    xsin=sine[xangle];
    xcos=sine[(xangle+kangle90) & kanglemask];

    ysin=sine[yangle];
    ycos=sine[(yangle+kangle90) & kanglemask];

    zsin=sine[zangle];
    zcos=sine[(zangle+kangle90) & kanglemask];
}