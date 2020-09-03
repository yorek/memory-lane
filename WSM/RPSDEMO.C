// File: RpsDemo.c
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>
#include <dos.h>
#include <math.h>
#include "rpsdemo.h"
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

/*-------------------------------------------------------------------------*/
/* COPIA IN VGA IL BUFFER PASSATO. SINCRONA CON IL RETRACE                 */
/*-------------------------------------------------------------------------*/
void BufToVga(BYTE *buf)
{
   _asm {
      push es
      push ds

      lds si,[buf]

      mov ax,0xa000
      mov es,ax
      xor di,di

      mov cx,16000

      mov dx,0x03da

wait_ret:
      in al,dx
      test al,0x08
      jnz wait_ret

      rep movsd

      pop ds
      pop es
   }
}

/*-------------------------------------------------------------------------*/
/* ASPETTA IL RETRACE                                                      */
/*-------------------------------------------------------------------------*/
void WaitRet(void)
{
   _asm {
      mov  dx,0x3da
L1:
      in   al,dx
      and  al,0x08
      jnz  L1
L2:
      in   al,dx
      and  al,0x08
      jz   L2
   };
}

/*-------------------------------------------------------------------------*/
/* COPIA IL VGA IL BUFFER SENZA SINCRONIA CON IL RETRACE                   */
/*-------------------------------------------------------------------------*/
void VgaToBuf(BYTE *dest)
{
   _asm {
      push  es
      push  ds

      mov   ax,0xA000
      mov   ds,ax
      xor   si,si

      les   di,[dest]

      mov   cx,16000
      rep   movsd

      pop   ds
      pop   es
   };
}

/*-------------------------------------------------------------------------*/
/* PULISCE WHERE DEL COLORE PASSATO                                        */
/*-------------------------------------------------------------------------*/
void ClearScreen(BYTE color, BYTE *where)
{
   _asm {
      push  es

      les   di,[where]

      mov   al,[color]
      mov   ah,al
      push  ax
      shl   eax,16
      pop   ax

      mov   cx,16000
      cld
      rep   stosd
      pop   es
   };
}

/*-------------------------------------------------------------------------*/
/* DISEGNA UNA LINEA ORIZZONTALE DA X1 A X2, ALLA Y, IN WHERE              */
/*-------------------------------------------------------------------------*/
void Hline(WORD x1, WORD x2, WORD y, BYTE color, BYTE *where)
{
   _asm {
      push  es

      les   di,[where]
      mov   ax,di
      push  ax

      mov   ax,[y]
      add   di,ax
      shl   ax,8
      shl   di,6
      add   di,ax
      add   di,[x1]

      pop   ax
      add   di,ax

      mov   al,[color]
      mov   ah,al
      mov   cx,[x2]
      sub   cx,[x1]
      shr   cx,1
      jnc   Start
      stosb

Start:
      rep stosw
      pop es
   };
}

/*-------------------------------------------------------------------------*/
/* DISEGNA UNA LINEA NUL BUFFER PASSATO                                    */
/*-------------------------------------------------------------------------*/
void Bline(WORD x1, WORD y1, WORD x2, WORD y2, BYTE color, BYTE *where)
{
   // Per il tracciamento della linea
   int i,deltax,deltay,numpix,d,dinc1,dinc2,x,xinc1,xinc2,y,yinc1,yinc2;

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
   x=x1;
   y=y1;

   for (i=0;i<numpix;i++) {
      PutPix(x,y,color,where);
      if (d<0) {
         d+=dinc1;
         x+=xinc1;
         y+=yinc1;
      } else {
         d+=dinc2;
         x+=xinc2;
         y+=yinc2;
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

   _asm {
      mov   si,OFFSET xlist
      mov   cx,200
FILLOOP:
      mov   ax,320
      mov   ds:[si],ax
      inc   si
      inc   si
      xor   ax,ax
      mov   ds:[si],ax
      inc   si
      inc   si
      loop  FILLOOP
   };

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
/* ACCENDO UN PIXEL IN WHERE DI COLORE NUMCOL                              */
/*-------------------------------------------------------------------------*/
void PutPix(WORD x, WORD y, BYTE numcol, BYTE *Where) {
   _asm {
      push  es

      les   di,[Where]
      mov   ax,di
      push  ax

      mov   ax,[y]
      mov   di,ax
      shl   ax,8
      shl   di,6
      add   di,ax
      add   di,[x]

      pop   ax
      add   di,ax

      xor   ah,ah
      mov   al,[numcol]
      stosb

      pop es
   };
}

/*-------------------------------------------------------------------------*/
/* LEGGO UN PIXEL IN WHERE                                                 */
/*-------------------------------------------------------------------------*/
BYTE GetPix(WORD x, WORD y, BYTE *Where) {
   BYTE col;

   _asm {
      push  es

      les   di,[Where]
      mov   ax,di
      push  ax

      mov   ax,[y]
      mov   di,ax
      shl   ax,8
      shl   di,6
      add   di,ax
      add   di,[x]

      pop   ax
      add   di,ax

      mov   al,es:[di]
      mov   [col],al
      pop es
   };

   return col;
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
/* LEGGE UN FILE .RPS                                                      */
/*-------------------------------------------------------------------------*/
// NESSUN OUTPUT SU SCHERMO E' EFFETTUATO IN QUESTA PROCEDURA.
// Il file viene messo in BUF e la palette in DESTPAL.
// Sar… necessario quindi poi copiare il buffer su schermo e quindi
// settare la palette video.

// [imgsize] Š la grandezza dell'imagine ESCLUSI header e tavolozza
// [buf] Š il buffe dove verr… messa l'imagine. DEVE ESSERE GIA ALLOCATO
// E VERIFICATO!!!!!
char LoadRps(char *filename, WORD imgsize, BYTE *buf)
{
   BYTE r,g,b;
   long int i;
   char header[17];
   FILE *fp;


   // apre il file
   if ((fp=fopen(filename,"rb"))==NULL) {
      fprintf(stderr,"Hey...non trovo un file!");
      return NULL;
   }

   // legge l'header dell'immagine
   if ((fread(header,sizeof(char),17,fp))!=17) {
      fprintf(stderr,"Errore di lettura dell'header del file!");
      fclose(fp);
      return NULL;
   }

   // legge i dati
   for (i=0L; i<imgsize; i++) {
      if ((fread(&buf[i],sizeof(char),1,fp))!=1) {
         fprintf(stderr,"Errore di lettura dei dati!");
         fclose(fp);
         return NULL;
      }
   }

   // legge e memorizza la palette
   for (i=0;i<256;i++) {
      r=fgetc(fp)>>2;
      g=fgetc(fp)>>2;
      b=fgetc(fp)>>2;
      destpal[i].r=r;
      destpal[i].g=g;
      destpal[i].b=b;
   }

   // chiude il file
   fclose(fp);
   return 1;
}

/*-------------------------------------------------------------------------*/
/* STAMPA UN LINEA DI TESTO                                                */
/*-------------------------------------------------------------------------*/
void Stampa(BYTE *where, char *mex, int y, WORD ct, BYTE smooth)
{
   WORD i,it,v,vt,c,r,x,xt,yt,k,ks;
   WORD col;

   x=0;
   x=(320-((strlen(mex)<<3)));
   x>>=1;

   ks=0;
   if (smooth==SMOOTH_OFF) ks=ct;

   for (k=ks; k<=ct; k++) {
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
               if (fnt8[vt+(c<<3)+r]==0) {
                  col=GetPix(xt,yt,where);
               } else {
                  if (smooth==SMOOTH_ON)
                     col=Smooth(xt,yt,k,where);
                  else
                     col=k;
               }

               PutPix(xt,yt,col,where);
            }
      }
   }
}

/*-------------------------------------------------------------------------*/
/* EFFETTUA LO SMOOTH DI UN PIXEL, A SECONDA DEI PIXEL INTORNO AD ESSO     */
/*-------------------------------------------------------------------------*/
/* "icol" e' il colore che si vuole assegnare al pixel.
   in usicta si trova il valore icol "trattato" in modo che sia in armonia
   con i colori che gli stanno intorno. */
BYTE Smooth(WORD x, WORD y, BYTE icol, BYTE *where)
{
   WORD col;

   col=(icol<<2);
   col=col+(GetPix(x+1,y,where)<<1);
   col=col+(GetPix(x-1,y,where)<<1);
   col=col+(GetPix(x,y+1,where)<<1);
   col=col+(GetPix(x,y-1,where)<<1);
   col+=GetPix(x-1,y-1,where);
   col+=GetPix(x+1,y-1,where);
   col+=GetPix(x-1,y+1,where);
   col+=GetPix(x+1,y+1,where);
   col>>=4;

   return (BYTE)col;
}

/*-------------------------------------------------------------------------*/
/* EFFETTUA LO SMOOTH DELL'INTERA MEMORIA WHERE                            */
/*-------------------------------------------------------------------------*/
void ScreenBlend(BYTE *where)
{
   /* la matrice usata e'
         0 1 0
         1 4 1
         0 1 0
      in luogo della vecchia
         1 2 1
         2 4 2
         1 2 1
      in quanto e' piu' veloce (31 fps contro 23 fps) e produce risultati
      pressoche' identici.
      Cio' permette anche di fare un unroll del loop centrale e raggiungere i 46 fps!!!!*/

   // Linea dello schermo da y=0
   _asm {
      push  es

      les   di,[where]

      // Punto con x=0, y=0
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl
      inc   di

      // Punti con x>0
      mov   cx,319

LoopXA:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl
      inc   di

      loop  LoopXA

   };

   // Linee dello schermo da y=>1 y<=198
   _asm {
      mov   cx,198

LoopYB:
      push  cx
      mov   cx,160

LoopXB:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di-320]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl
      inc   di

      // Unrolled 1 time

      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di-320]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl
      inc   di

      loop  LoopXB

      pop   cx
      loop  LoopYB
    };

   // Riga dello schermo con y=199
   // e x>=0 x<=318
   _asm {
      mov   cx,319

LoopXC:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di-320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl
      inc   di

      loop  LoopXC

      // Punto con x=319, y=199
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di-320]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl

      pop es
   };

}

/*-------------------------------------------------------------------------*/
/* FIRE EFFECT!!!                                                          */
/*-------------------------------------------------------------------------*/
void DoFire(BYTE *where)
{
   // E' un caso particolare della routine sopra.

   // Linea dello schermo da y=0
   _asm {
      push  es

      les   di,[where]

      // Punto con x=0, y=0
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,2

      mov   es:[di],bl
      inc   di

      // Punti con x>0
      mov   cx,319

LoopXA:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,2

      mov   es:[di],bl
      inc   di

      loop  LoopXA
   };

   // Linee dello schermo da y=>1 y<=198
   _asm {
      mov   cx,198

LoopYB:
      push  cx
      mov   cx,160

LoopXB:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,2
      jz    n1

      dec   bl
      mov   es:[di],bl
n1:
      inc   di

      // Unrolled 1 time
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      mov   al,es:[di+320]
      add   bx,ax

      shr   bx,2
      jz    n2

      dec   bl
      mov   es:[di],bl

n2:
      inc   di
      loop  LoopXB

      pop   cx
      loop  LoopYB
    };

   _asm {
      mov   cx,319

LoopXC:
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]

      mov   al,es:[di+1]
      add   bx,ax

      mov   al,es:[di-1]
      add   bx,ax

      shr   bx,2

      mov   es:[di],bl
      inc   di

      loop  LoopXC

      // Punto con x=319, y=199
      xor   ax,ax
      xor   bx,bx

      mov   bl,es:[di]
      shl   bx,2

      mov   al,es:[di-1]
      add   bx,ax

      shr   bx,3

      mov   es:[di],bl

      pop es
   };

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
