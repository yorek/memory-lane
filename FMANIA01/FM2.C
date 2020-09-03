//
// Fractal mania demo
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <i86.h>
#include "write.h"
#include "vga.h"

// Vga
#define vga 0x0A0000

// Centrano il frattale
#define LY 36
#define LX 96

// Aumento dell'oribita valori.
#define AO 0.002

void Mania(char mode);
void Bye(void);
void SetColors(void);
int Fract_1(void);
int Fract_2(void);
int Fract_Map(void);
void Mandel(int ax, int ay, int bx, int by);
char InitMem(void);
void FreeMem(void);

unsigned char *buffer   =NULL;
unsigned char *map      =NULL;
unsigned short int *yval=NULL;

int main(int argc, char *argv[])
{
    union REGS r;
    struct SREGS sr;
    char *systems[] = {"Raw", "XMS", "VCPI", "DPMI"};
    char *processor[] = {"80386", "80486", "80586","Unknown"};

    r.x.eax = 3;
    int386 (0x10, &r, &r);

    segread (&sr);
    r.w.ax = 0xEEFF;
    int386x (0x31, &r, &r, &sr);

    printf("Fractal Mania! v0.1 (c)1998 Davide Mauri\n\n");

    if (r.x.cflag) {
        printf ("Extender Info Function Not Present!\n");
        exit (1);
    }

    printf ("System Type        : %s\n", systems[r.h.ch]);

    if (r.h.cl > 5) r.h.cl = 6;
    printf ("Processor          : %s (Recommended: 586) \n\n", processor[r.h.cl-3]);

    if ( argc==1 )
    {
        Mania(1);
        return(0);
    }
    if ( argc > 1 )
    {
        if ( argv[1][0]=='/' && argv[1][1]=='e' )
        {
            Mania(3);
            return(0);
        }
        if ( argv[1][0]=='/' && argv[1][1]=='m' )
        {
            Mania(4);
            return(0);
        }
        if ( argv[1][0]=='/' && argv[1][1]=='?' )
        {
            printf("Parameters:\n");
            printf("[/e] extrude image (Not real 3d, for now).\n");
            printf("[/m] mandelbrot set. (Static).\n");
            return(0);
        }
    }

    printf("Fractal Mania! 0.1 (c)1998 by Davide Mauri\n");
    printf("Wrong parameter. Use /? for help\n\n");
    return(1);
}


//
// Principale
//
void Mania(char mode)
{
    union REGS rg;
    clock_t start,end;
    int i,frames;

    //
    // Alloca memoria
    //
    if (InitMem()==0)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    //
    // Crea la tabella di lookup per l'offset
    //
    for ( i=0; i<200; i++ )
    {
        yval[i]=i*320;
    }

    //
    // Attiva modalita' grafica
    //
    rg.w.ax=0x13;
    int386(0x10,&rg,&rg);

    //
    // Setta i colori
    //
    SetColors();

    //
    // Cencella buffer
    //
    memset(buffer,0,64000);

    //
    // Via!
    //
    if ( mode==1 )
    {
        start=clock();
        frames=Fract_1();
        end=clock();
    }
    if ( mode==3 )
    {
        start=clock();
        frames=Fract_Map();
        end=clock();
    }
    if ( mode==4 )
    {
        Mandel(0,0,127,127);
        getch();
    }

    //
    // Attiva modalita' testo
    //
    rg.w.ax=0x03;
    int386(0x10,&rg,&rg);


    //
    // Un po' di dati
    //
    printf("Frames : %ld\n",frames);
    printf("Time   : %3.3f secs\n",(float)(end-start)/1000.0);
    printf("FPS    : %3.3f\n",(float)(frames*1000.0/(end-start)));

    //
    // Messaggio finale
    //
    Bye();

    //
    // Libera memoria
    //
    FreeMem();
}

//
// Alloca memoria
//
char InitMem(void)
{
    int temp;

    temp=320*200;
    buffer=malloc(temp);
    if (buffer==NULL) return 0;

    temp=200*sizeof(unsigned short int);
    yval=malloc(temp);
    if (yval==NULL) return 0;

    temp=128*128;
    map=malloc(temp);
    if (map==NULL) return 0;

    return 1;
}

//
// Libera memoria
//
void FreeMem(void)
{
    if ( buffer!=NULL ) free(buffer);
    if ( map!=NULL ) free(map);
    if ( yval!=NULL ) free(yval);
}

//
// Messaggio finale
//
void Bye(void)
{
    printf("\n\nFractal Mania! 0.1 (c) 1998 by Davide Mauri.\n");
    printf("Visit my page at:\n");
    printf("   www.geocities.com\\SouthBeach\\Docks\\9426  \n");
    printf("\nIf you want contact me please send an e-mail to:\n");
    printf("   manowar-rps@usa.net \n");
    printf("   or me2342@mclink.it \n");
}

//
// Setta la palette
//
void SetColors(void)
{
    int i;
    for ( i=0; i<64; i++ )
    {
        outp(0x3C8,i);
        outp(0x3C9,0);
        outp(0x3C9,0);
        outp(0x3C9,i);

        outp(0x3C8,64+i);
        outp(0x3C9,i);
        outp(0x3C9,0);
        outp(0x3C9,63-i);

        outp(0x3C8,128+i);
        outp(0x3C9,63);
        outp(0x3C9,i);
        outp(0x3C9,0);

        outp(0x3C8,192+i);
        outp(0x3C9,63);
        outp(0x3C9,63);
        outp(0x3C9,i);
    }
}

//
// Standard
//
int Fract_1(void)
{
    int x,y;
    int xp,yp;
    float x0,y0,x1,y1;
    float xs,ys;
    float r,c;
    int frames;
    int i;
    float ang,ang2,ic;
    char txt[80];

    r=0;
    c=0;
    frames=0;
    ic=0.01;

    //
    // Mandelbrot
    //
    for ( x=0; x<=160; x++ )
    {
        xs=-2.0+x/40.0;
        for ( y=0; y<=160; y++ )
        {
            ys=2.0-y/40.0;
            x0=xs;
            y0=ys;
            for ( i=0; i<255; i++)
            {
                x1=x0*x0-y0*y0+xs;
                y1=2*x0*y0+ys;

                if ((x1*x1 + y1*y1)>4) break;
                x0=x1;
                y0=y1;
            }
            buffer[160+x+y*320]=i;
        }
    }

    //
    // Julia + c,r orbit value
    //
    do
    {
        memset(buffer+51200,0,12480);

        for ( x=0; x<159; x++ )
        {
            xs=-2.0+x/40.0;
            for ( y=0; y<159; y++ )
            {
                ys=2.0-y/40.0;
                x0=xs;
                y0=ys;
                for ( i=0; i<=248; i+=5)
                {
                    x1=x0*x0-y0*y0+r;
                    y1=2*x0*y0+c;

                    if ((x1*x1 + y1*y1)>4) break;
                    x0=x1;
                    y0=y1;
                }
                buffer[x+y*320]=i;
            }
        }

        //
        // Change c and r every cycle
        //
        c=0.8*cos(ang)+0.25*cos(ang2);
        r=0.8*sin(ang)+c*sin(ang2);

        //
        // Orbits
        //
        xp=80+c*40;
        if ( xp>160 ) xp=160;
        if ( xp<0 ) xp=0;

        yp=80+r*40;
        if ( yp>160 ) yp=160;
        if ( yp<0 ) yp=0;

        buffer[159+xp+yp*320]=128;

        ang=ang+0.01;
        if ( ang>6.283185 ) ang=ang-6.283185;

        ang2=ang2+0.007;
        if ( ang2>6.283185 ) ang2=ang2-6.283185;

        sprintf(txt,"Z   : %f + %fi",c,r);
        Write4(buffer,txt,3,170,255);

        //sprintf(txt,"Ang : %f (%f)",ang,(57.2958*ang));
        //Write4(buffer,txt,3,180,255);

        //sprintf(txt,"Ang2: %f (%f)",ang2,(57.2958*ang2));
        //Write4(buffer,txt,3,190,255);

        frames++;
        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// Mandlebrot full-screen
//
void Mandel(int ax, int ay, int bx, int by)
{
    int x,y;
    float x0,y0,x1,y1;
    float xs,ys;
    int i;

    //
    // Mandelbrot
    //
    for ( x=0; x<320; x++ )
    {
        xs=-2.0+x/120.0;
        for ( y=0; y<200; y++ )
        {
            ys=1.0-y/100.0;
            x0=xs;
            y0=ys;
            for ( i=0; i<255; i++)
            {
                x1=x0*x0-y0*y0+xs;
                y1=2*x0*y0+ys;

                if ((x1*x1 + y1*y1)>4) break;
                x0=x1;
                y0=y1;
            }
            buffer[x+y*320]=i;
        }

        memcpy(vga,buffer,64000);
    }
}

//
// Estrusione
//
int Fract_Map(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i,j,l;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        memset(buffer,0,64000);
        a=LX;
        b=LX+128;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY+22;
            b2=LY+22+128;
            aid=x0*x0;
            for ( y=0; y<=64; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                j=0;
                for ( i=0; i<=248; i+=6)
                {
                    j++;
                    x1=(lkx-lky+r)>>8;

                    y1=lkxy;
                    y1+=y1;
                    y1+=c;
                    y1>>=8;

                    lkx=x1*x1;
                    lky=y1*y1;
                    if ((lkx | lky)>262144) break;
                    lkxy=x1*y1;
                }

                if ( i!=0 )
                {
                    l=buffer[a+yval[a2-j]];
                    if (l<i) buffer[a+yval[a2-j]]=(l+i) > 255 ? 255 : (l+i);

                    l=buffer[b+yval[b2-j]];
                    if (l<i) buffer[b+yval[b2-j]]=(l+i) > 255 ? 255 : (l+i);

                }

                y0+=8;
                a2++;
                b2--;
            }
            x0+=8;
            a++;
            b--;
        }

        c=55535*sin(ang)+16384*cos(ang2);
        r=55535*cos(ang)+c*sin(ang2);

        count++;
        ang=ang+ic;
        ang2=ang2+0.007;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }

        frames++;
        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}
