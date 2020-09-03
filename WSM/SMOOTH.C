#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include "rpsdemo.h"

BYTE *buf;

void main (void)
{
   int i;
   int x,y;
   clock_t startTime,stopTime;
   double fps = 0.0;

   vga=(BYTE *)MK_FP(0xA000,0x0);
   buf=malloc(64000);
   printf("Smooth by Grizly of Rolling Pixels.\n\n");
   printf("VGA ptr: %p\n",vga);
   printf("Buf ptr: %p\n",buf);
   if (buf==NULL) exit(1);
   printf("Generating VIRTUAL sine table...");
   CalculateSine();
   printf("Done.\n");
   getch();

   VideoMode(0x13);
   ClearScreen(0,vga);
   ClearScreen(0,buf);

   for (i=0;i<64;i++) {
      SetColor(1+i,     0,       i,       i);
      SetColor(65+i,    i,       63,      63);
      SetColor(129+i,   63,      63-i,    63-i);
      SetColor(192+i,   63-i,    0,       0);
   }
   GetPalette();

   for (i=0; i<256; i++) {
      x=sine[i];
      x>>=2;

      y=sine[(i+kangle90) & kanglemask];
      y>>=2;

      Bline(160-x,100-y+x, x+160+y,y+100, i,buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,i>>1,SMOOTH_OFF);
      ScreenBlend(buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,i,SMOOTH_OFF);
      BufToVga(buf);
  }

   for (i=0; i<256; i++) {
      x=sine[i];
      x>>=2;

      y=sine[(i+kangle90) & kanglemask];
      y>>=2;

      Bline(160-x,100-y+x, x+160+y,y+100, 255,buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,128,SMOOTH_OFF);
      ScreenBlend(buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,255,SMOOTH_OFF);
      BufToVga(buf);
   }

   for (i=0; i<256; i++) {
      x=sine[i];
      x>>=2;

      y=sine[(i+kangle90) & kanglemask];
      y>>=2;

      Bline(160-x,100-y+x, x+160+y,y+100, 255-i,buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,127-(i>>1),SMOOTH_OFF);
      DoFire(buf);

      Stampa(buf,"SMOOTH (C)1997 CODE BY GRIZLY",95,255-(i>>1),SMOOTH_OFF);
      BufToVga(buf);
   }

   Fade_To_Black(0,255);
   ClearScreen(0,vga);

   LoadRps("backgnd.rps",64000L,buf);
   BufToVga(buf);
   Fade_In();

   startTime=clock();
   for (i=0; i<92; i++) {
      ScreenBlend(buf);
      BufToVga(buf);
   }
   stopTime=clock();

   Fade_To_Black(0,255);
   ClearScreen(0,vga);
   ClearScreen(0,buf);

   for (i=0;i<64;i++) {
      SetColor(1+i,i/2,0,0);
      SetColor(65+i,32+i/2,i/2,0);
      SetColor(129+i,63,32+i/2,i/2);
      SetColor(192+i,63,63,32+i/2);
   }
   GetPalette();

   for (i=0; i<256; i++) {
      x=sine[i & kanglemask];
      x>>=2;

      y=sine[(i+kangle90) & kanglemask];
      y>>=2;

      Bline(160-x,100-y+x, 160+x+y,y+100+x, 255,buf);
      Bline(160+x,100-y+x, 160-x-y,y+100+x, 255,buf);

      Stampa(buf,"ROLLING PIXELS (C) 1997",95,192,SMOOTH_OFF);
      DoFire(buf);

      Stampa(buf,"ROLLING PIXELS (C) 1997",95,255,SMOOTH_OFF);
      BufToVga(buf);
   }

   for (i=0; i<256; i++) {
      x=sine[i & kanglemask];
      x>>=2;

      y=sine[(i+kangle90) & kanglemask];
      y>>=2;

      Bline(160-x,100-y+x, 160+x+y,y+100+x, 255-i,buf);
      Bline(160+x,100-y+x, 160-x-y,y+100+x, 255-i,buf);

      DoFire(buf);

      Stampa(buf,"ROLLING PIXELS (C) 1997",95,255-i,SMOOTH_OFF);
      BufToVga(buf);
   }


   Fade_To_Black(0,255);
   VideoMode(0x03);

   free(buf);

   fps = 92.0  / (double)((stopTime - startTime) / CLOCKS_PER_SEC) ;
   printf("Frames are 92.\n");
   printf("Time to complete  : %16d secs\n",((stopTime - startTime) / CLOCKS_PER_SEC));
   printf("Time to complete  : %16ld ticks\n",(stopTime - startTime));
   printf("Average frame rate: %16.7f fps\n", fps);
}