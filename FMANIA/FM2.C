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

// Vga
#define vga 0x0A0000

// Centrano il frattale
#define LY 36
#define LX 96

// Aumento dell'oribita valori.
#define AO 0.002
#define A1 0.007

unsigned char *buffer;
unsigned short int *yval;

//
// Aspetta il retrace verticale.
//
void WairRet(void);
#pragma aux WaitRet =    \
   "mov dx, 0x03da"      \
   "@loop1:"             \
   "in  al, dx"          \
   "and al, 0x08"        \
   "jnz @loop1"          \
   "@loop2:"             \
   "in  al, dx"          \
   "and al, 0x08"        \
   "jz @loop2"           \
   modify [dx al];

//
// "Sfuoca" lo schermo
//
void ScreenBlend (unsigned char *dest, unsigned char *where);
#pragma aux ScreenBlend =       \
    "add    esi,640"            \
    "add    edi,640"            \
    "mov    ecx,62720"          \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "LoopB:"                    \
        "mov    al,[esi]"           \
        "shl    ax,2"               \
        "mov    bl,[esi+640]"       \
        "add    ax,bx"              \
        "mov    bl,[esi+2]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-2]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-640]"       \
        "add    ax,bx"              \
        "shr    ax,3"               \
        "mov    [edi],al"           \
        "inc    edi"                \
        "inc    esi"                \
    "dec    ecx"                \
    "jnz    LoopB"              \
    parm [edi][esi]             \
    modify [eax ebx ecx];

//
// Bilinear filtering
//
void ScreenBlur (unsigned char *dest, unsigned char *src);
#pragma aux ScreenBlur =      \
    "add    esi,320"            \
    "add    edi,320"            \
    "mov    ecx,63360"          \
    "LoopB:"                    \
        "xor    ax,ax"              \
        "xor    bx,bx"              \
        "mov    al,[esi]"           \
        "shl    ax,2"               \
        "mov    bl,[esi+320]"       \
        "add    ax,bx"              \
        "mov    bl,[esi+1]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-1]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-320]"       \
        "add    ax,bx"              \
        "shr    ax,3"               \
        "inc    esi"                \
        "mov    [edi],al"           \
        "inc    edi"                \
    "dec    ecx"                \
    "jnz    LoopB"              \
    parm [edi][esi]             \
    modify [eax ebx ecx];


void Mania(char mode);
void Bye(void);
void SetColors(void);
int Fract_1(void);
int Fract_2(void);
int Fract_3(void);
int Fract_4(void);
int Fract_5(void);
int Fract_6(void);

int main(int argc, char *argv[])
{
    union REGS r;
    struct SREGS sr;
    char far *copyright;
    char *systems[] = {"Raw", "XMS", "VCPI", "DPMI"};
    char *processor[] = {"80386", "80486", "80586","Unknown"};

    r.x.eax = 3;
    int386 (0x10, &r, &r);

    segread (&sr);
    r.w.ax = 0xEEFF;
    int386x (0x31, &r, &r, &sr);

    if (r.x.cflag) {
        printf ("Extender Info Function Not Present!\n");
        exit (1);
    }

    copyright = MK_FP (sr.es, r.x.ebx);

    printf ("Copyright String:\n");
    printf ("-----------------\n");
    printf ("%s\n\n", copyright);

    printf ("System Information:\n");
    printf ("-------------------\n");

    printf ("Extender Code      : %.6Xh ", r.x.eax);
    printf ("(%c%c%c%c)\n", r.x.eax >> 24, r.x.eax >> 16, r.x.eax >> 8,
            r.x.eax);
    printf ("Extender Version   : v%d.%d\n", r.h.dh, r.h.dl);

    printf ("System Type        : %s\n", systems[r.h.ch]);

    if (r.h.cl > 5) r.h.cl = 6;
    printf ("Processor          : %s (Recommended: 586) \n\n", processor[r.h.cl-3]);

    printf ("Program Information:\n");
    printf ("--------------------\n");
    if ( argc==1 )
    {
        Mania(1);
        return(0);
    }
    if ( argc > 1 )
    {
        if ( argv[1][0]=='/' && argv[1][1]=='w' )
        {
            Mania(2);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='o' )
        {
            Mania(3);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='b' )
        {
            Mania(4);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='f' )
        {
            Mania(5);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='a' )
        {
            Mania(6);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='j' )
        {
            Mania(7);
            return(0);
        }

        if ( argv[1][0]=='/' && argv[1][1]=='?' )
        {
            printf("Fractal Mania! v1.2b (c)1999 Davide Mauri\n");
            printf("Parameters:\n");
            printf("   [/a] use a more accurate algorithm (of course this sucks a little fps).\n");
            printf("   [/w] use a more accurate algorithm and render a more BIG area.\n");
            printf("   [/b] draw images using bilinear interpolation.\n");
            printf("   [/f] fake bumping (Was an experiment...)\n");
            printf("   [/o] draw orbits of values used to generate fractals.\n");
            printf("   [/j] draw the Julibrot set (Cooool!).\n");
            printf("   [/?] this help.\n");
            return(0);
        }
    }

    printf("Fractal Mania! v1.2b (c)1999 Davide Mauri\n");
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
    buffer=malloc(320*200);
    yval=malloc(200*sizeof(unsigned short int));
    if ( buffer==NULL || yval==NULL )
    {
        printf("Unable to allocate bare memory for demo. BYE!\n");
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
    Write20(buffer,"Fractal Mania",88,10,255);
    Write4(buffer,"A little demo to show you",106,170,255);
    Write4(buffer,"the amazing world of fractals!",96,180,255);
    ScreenBlur(buffer,buffer);

    //
    // Via!
    //
    if ( mode==1 )
    {
        start=clock();
        frames=Fract_1();
        end=clock();
    }
    if ( mode==2 )
    {
        start=clock();
        frames=Fract_2();
        end=clock();
    }
    if ( mode==3 )
    {
        start=clock();
        frames=Fract_3();
        end=clock();
    }
    if ( mode==4 )
    {
        start=clock();
        frames=Fract_4();
        end=clock();
    }
    if ( mode==5 )
    {
        start=clock();
        frames=Fract_5();
        end=clock();
    }
    if ( mode==6 )
    {
        start=clock();
        frames=Fract_6();
        end=clock();
    }
    if ( mode==7 )
    {
        start=clock();
        frames=Fract_7();
        end=clock();
    }

    //
    // Cancella lo schermo
    //
    for ( i=0; i<256; i++ )
    {
        ScreenBlend(buffer,buffer);
        WaitRet();
        memcpy(vga,buffer,64000);
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
    // Libera memoria
    //
    free(buffer);
    free(yval);
}

//
// Messaggio finale
//
void Bye(void)
{
    printf("\n\nFractal Mania! v1.2b (c)1999 by Davide Mauri.\n");
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
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        memset(buffer+11200,0,41280);

        a=LX;
        b=LX+128;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY;
            b2=LY+128;
            aid=x0*x0;
            for ( y=0; y<=64; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=8)
                {
                    x1=(lkx-lky+r)>>8;

                    y1=lkxy;
                    y1+=y1;
                    y1+=c;
                    y1>>=8;

                    lkx=x1*x1;
                    lky=y1*y1;
                    if ((lkx + lky)>262144) break;
                    lkxy=x1*y1;
                }
                if ( i!=0 )
                {
                    buffer[a+yval[a2]]=i;
                    buffer[b+yval[b2]]=i;
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

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }
        //WaitRet();
        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// Like Standard but more accurate and bigger!
//
int Fract_2(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i;
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

        a=62;
        b=260;
        x0=-495;
        for ( x=0; x<=198; x++ )
        {
            y0=-495;
            a2=0;
            b2=198;
            aid=x0*x0;
            for ( y=0; y<=99; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=6)
                {
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
                    buffer[a+yval[a2]]=i;
                    buffer[b+yval[b2]]=i;
                }
                y0+=5;
                a2++;
                b2--;
            }
            x0+=5;
            a++;
            b--;
        }

        c=55535*sin(ang)+16384*cos(ang2);
        r=55535*cos(ang)+c*sin(ang2);

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}


//
// Disegna l'orbita
//
int Fract_3(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i,j;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    j=16;
    do
    {
        buffer[160+(r>>10)+yval[100+(c>>10)]]=j;

        c=55535*sin(ang)+16384*cos(ang2);
        r=55535*cos(ang)+c*sin(ang2);

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            j+=4;
            if ( j>255 ) j=16;

            ic=ic+AO;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// bilinear filtering
//
int Fract_4(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i,j;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        a=LX;
        b=LX+128;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY;
            b2=LY+128;
            aid=x0*x0;
            for ( y=0; y<=64; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=8)
                {
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
                j=a+yval[a2];
                buffer[j]=(((i<<2)+buffer[j+1]+buffer[j-1]+buffer[j+320]+buffer[j-320])>>3);

                j=b+yval[b2];
                buffer[j]=(((i<<2)+buffer[j+1]+buffer[j-1]+buffer[j+320]+buffer[j-320])>>3);

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

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// fake bumping
//
int Fract_5(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i,j;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        a=LX;
        b=LX+128;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY;
            b2=LY+128;
            aid=x0*x0;
            for ( y=0; y<=64; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=8)
                {
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
                j=a+yval[a2];
                buffer[j]=(((i<<2)+buffer[j+1]+buffer[j+1]+buffer[j+320]+buffer[j-320])>>3);

                j=b+yval[b2];
                buffer[j]=(((i<<2)+buffer[j+1]+buffer[j+1]+buffer[j+320]+buffer[j-320])>>3);
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

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+0.001;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// Like Standard but more accurate!
//
int Fract_6(void)
{
    int x,y;
    int x0,y0,x1,y1,a,b,a2,b2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        memset(buffer+11200,0,41280);

        a=LX;
        b=LX+128;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY;
            b2=LY+128;
            aid=x0*x0;
            for ( y=0; y<=64; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=6)
                {
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
                    buffer[a+yval[a2]]=i;
                    buffer[b+yval[b2]]=i;
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

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}

//
// Julibrot
//
int Fract_7(void)
{
    int x,y;
    int x0,y0,x1,y1,a,a2;
    int r,c;
    int frames;
    int lkx,lky,lkxy;
    int aid;
    int i;
    float ang,ang2,ic;
    int count;

    r=c=0;
    frames=0;
    ang=ang2=0;
    ic=0.001;
    count=0;
    do
    {
        memset(buffer+11200,0,41280);

        a=LX;
        x0=-512;
        for ( x=0; x<=128; x++ )
        {
            y0=-512;
            a2=LY;
            aid=x0*x0;
            for ( y=0; y<128; y++ )
            {
                lkx=aid;
                lky=y0*y0;
                lkxy=x0*y0;
                for ( i=0; i<248; i+=5)
                {
                    x1=(lkx-lky+r+(x0<<7))>>8;

                    y1=lkxy;
                    y1+=y1;
                    y1+=c+(y0<<7);
                    y1>>=8;

                    lkx=x1*x1;
                    lky=y1*y1;
                    if ((lkx + lky)>262144) break;
                    lkxy=x1*y1;
                }
                if ( i!=0 )
                {
                    buffer[a+yval[a2]]=i;
                }
                y0+=8;
                a2++;
            }
            x0+=8;
            a++;
        }

        c=55535*sin(ang)+16384*cos(ang2);
        r=55535*cos(ang)+c*sin(ang2);

        frames++;
        count++;
        ang=ang+ic;
        ang2=ang2+A1;

        if ( count==6290 )
        {
            count=0;
            ic=ic+AO;
        }

        memcpy(vga,buffer,64000);

    } while ( kbhit()==0 );
    return frames;
}
