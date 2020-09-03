#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include <i86.h>
#include <time.h>
#include <math.h>

#include "gfx32.h"
#include "palette.h"
#include "pcx.h"

#include "midas.h"
#include "midasdll.h"
#include "vu.h"

#define MODULEFILE "song.mod"

// Lens effect
#define DIMENSIONE      100
#define ZOOM            DIMENSIONE / 4

// Stars and Cube
#define NUMSTARS        200
#define NUMPOINTS       512
#define VEL             50

typedef struct {
    int x;
    int y;
    int z;
} STAR_STRUCT ;
STAR_STRUCT stars[NUMPOINTS];
STAR_STRUCT stella[5];

void Show_Intro(void);
void Do_Wind(BYTE *img, BYTE *where, int angle, int phase, int shiny);
void Do_Flame(BYTE *where, BYTE min, BYTE max);
void Do_Bump(BYTE *dest, BYTE *src, int lx, int ly);
void Do_Bump_2_Image(BYTE *dest, BYTE *src, BYTE *src2, int lx, int ly);

void Init_Stars_Structure(void);
void Do_Stars(BYTE *where);
void Init_Penta_Structure(void);
void Do_Rotating_5_Star(BYTE col, BYTE *where);
void Init_Cube_Structure(void);
void Do_Cube(int x, int y, BYTE *where);
void Build_Lens_Trasf_Matx(int d, int m);
void Do_Lens(BYTE *dest, BYTE *src, int lx, int ly);

int CALLING MakeMeter(sdSample *sdsmp, gmpSample *gmpsmp);
void ShowVuMeters(BYTE *where);

void Fine(char *msg);

int dummy;
int numChannels;
static unsigned meter;
static unsigned position, volume, sample;
static unsigned long rate;

int scrx=320;
int scry=200;
short int *mtx=NULL;
BYTE *colmtx=NULL;
BYTE *buf=NULL, *tmp=NULL, *tmp2=NULL, *img=NULL, *img2=NULL;
RGB *loadpal=NULL;
BYTE *trsp=NULL;

unsigned int oldCount=0;
volatile unsigned int frameCount=0;
void MIDAS_CALL PreVr(void)
{
    frameCount++;
}

void main(int argc, char *argv[])
{
    int t,t2,l,c,r,tr,off;
    int i,j,di,dj,error;
    long start,elapsed;
    static gmpModule *module;
    RGB *loadpal;
    BYTE cinemascope=0;
    int nosound;
    DWORD drate;

    if ( argc>1 )
    {
        if (( argv[1][0]=='C' ) || (argv[1][0]=='c'))
        {
            cinemascope=1;
        }
    }

    MIDASstartup();
    MIDASconfig();

    printf("\nAshes Demo BETA low-res version\n");
    printf("by Rolling Pixels (C) %s\n",__DATE__);
    printf("--> WATCH OUT FOR THE HIRES VERSION, WILL SOON BE RELEASED! <--\n");

    #ifdef __WATCOMC__
         printf("Using 32-bit compiler Watcom C version %d.%d.\n",
         (__WATCOMC__)/100,(__WATCOMC__%100));
    #endif

    printf("\nGFX version in use: %s\n", VERSION);
    printf("Dated: %s\n\n",DATE);
    vga=(BYTE *)0x0A0000;

    printf("Allocating memory : ");
    buf=malloc(MCGA);
    if ( buf==NULL ) Fine("*buf: Error allocating memory.\n");
    printf("þ");

    tmp=malloc(MCGA);
    if ( tmp==NULL ) Fine("*tmp: Error allocating memory.\n");
    printf("þ");

    tmp2=malloc(MCGA);
    if ( tmp2==NULL ) Fine("*tmp2: Error allocating memory.\n");
    printf("þ");

    img=malloc(16384);
    if ( img==NULL ) Fine("*img: Error allocating memory.\n");
    printf("þ");

    img2=malloc(16384);
    if ( img2==NULL ) Fine("*img2: Error allocating memory.\n");
    printf("þ");

    trsp=malloc(17600);
    if ( trsp==NULL ) Fine("*trsp: Error allocating memory.\n");
    printf("þ");

    loadpal=malloc(768);
    if ( loadpal==NULL ) Fine("*loadpal: Error allocating memory.\n");
    printf("þ");

    colmtx=malloc(DIMENSIONE * DIMENSIONE);
    if ( colmtx==NULL ) Fine("*colmtx: Error allocating memory.\n");
    printf("þ");

    mtx=malloc(DIMENSIONE * DIMENSIONE * sizeof(short int));
    if ( mtx==NULL ) Fine("*mtx: Error allocating memory.\n");
    printf("þ Ok.\n");

    drate=35000;
    printf("Refresh Display Rate: ");
    printf("%ld Hz.\n",drate/1000);

    printf("Calculating Sine lookup table...");
    CalculateSine();
    printf("%d values.\n",kangle360);

    printf("Building lens transformation matrix : ");
    Build_Lens_Trasf_Matx(DIMENSIONE, ZOOM);

    printf("Calculating 3D Datas....\n");
    Init_Penta_Structure();

    printf("Initializing FX and Music...\n");
    MIDASinit();
    printf("Using %s\n%s, using port %X, IRQ %i and DMA %i\n",
        midasSD->name, midasSD->cardNames[midasSD->cardType-1],
        midasSD->port, midasSD->IRQ, midasSD->DMA);
    if ( midasSD->port==0 ) nosound=1;
    else nosound=0;

    if ( (error=vuInit()) != OK )
        Fine("Unable to initialize VuMeters...\n");

    if ( (error=gmpLoadMOD(MODULEFILE,1,&MakeMeter,&module)) != OK )
        Fine("Unable to load mod...\n");
    numChannels=module->numChannels;

    if (MIDASsetTimerCallbacks(drate, FALSE, &PreVr, NULL, NULL)==FALSE)
        Fine("Cannot install callback timer.\n");
    else
        printf ("Callback timer installed.\n");

    if (cinemascope==1)
        printf("CINEMASCOPE switched on.\n");

    printf("\nHit any key to start....\n");
    getch();

    // Set video mode 320x200x256
    if (cinemascope==0)
    {
        VideoMode(0x13);
    } else {
        _asm {
            mov ax,0x13
            int 10h
            mov dx,0x03c2
            mov al,0x0e3
            out dx,al
            mov dx,0x3c6
            mov ax,0xff
            out dx,al
        }
    }
    GetPalette();

    if ( nosound==0) midasPlayModule(module,0);

    ClearScreen(0,vga);
    ClearScreen(0,buf);
    ClearScreen(0,tmp);

    Show_Intro();
    oldCount=frameCount;
    while ( frameCount==oldCount )
    Fade_to_white(0,255);
    ClearScreen(0,vga);

    // Carica il buffer trasparente
    LeggiPcx("Trasp3.pcx",trsp,loadpal);

    // Carica sfondo
    LeggiPcx("Init3.pcx",buf,loadpal);

    FlipMem(tmp,buf);
    SetPal1();
    Fade_In();

    tr=0;
    off=0;
    for ( r=0; r<200; r++)
    {
        for ( c=0; c<88; c++ )
        {
            off=c+(r<<8)+(r<<6);
            l=buf[off];
            l+=trsp[tr];
            if ( l>255 ) l=255;
            buf[off]=l;
            tr++;
        }
        oldCount=frameCount;
        while ( frameCount==oldCount )
        FlipMem(tmp2,buf);
        BufToVga(tmp2);
    }

    FlipMem(buf,tmp);
    for ( i=0; i<256; i++ )
    {
        ScreenBlend(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    Init_Cube_Structure();
    zoff=-256;
    for ( i=420; i>-100; i-- )
    {
        ClearScreen(0,buf);
        UpdateRotation(1,1,1);
        Do_Cube(i,100,buf);
        ShowVuMeters(buf);
        FastTrasp(buf,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    ClearScreen(255,vga);
    ClearScreen(0,buf);
    ClearScreen(0,tmp);
    ClearScreen(0,tmp2);
    Fade_To_Black(0,255);

    SetFirePal1();
    zoff=-384;
    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_in();
        UpdateRotation(1,2,3);
        Do_Rotating_5_Star(255,buf);
        Do_Flame(buf,64,191);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<256; i++ )
    {
        UpdateRotation(1,3,2);
        Do_Rotating_5_Star(255,buf);
        Do_Flame(buf,64,191);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<64; i++ )
    {
        UpdateRotation(2,2,1);
        Do_Rotating_5_Star(255,buf);
        Do_Flame(buf,64-i,191+i);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<252; i++ )
    {
        UpdateRotation(3,1,2);
        Do_Rotating_5_Star(255,buf);
        Do_Flame(buf,0,2);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    SetPal2();
    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_in();
        UpdateRotation(3,2,1);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<256; i++ )
    {
        zoff++;
        UpdateRotation(3,2,1);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<256; i++ )
    {
        UpdateRotation(1,3,2);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    ClearScreen(255,vga);
    LeggiPcx("Ashes3.pcx",tmp,loadpal);

    for ( i=0; i<256; i++ )
    {
        FlipMem(buf,tmp);
        UpdateRotation(2,2,1);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        ShowVuMeters(buf);
        FastTrasp(buf,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    ClearScreen(0,buf);
    FlipMem(buf,tmp);
    for ( i=0; i<232; i++ )
    {
        UpdateRotation(3,2,1);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( ; i<256; i++ )
    {
        UpdateRotation(2,2,2);
        Do_Rotating_5_Star(255,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<192; i++ )
    {
        UpdateRotation(2,1,3);
        Do_Rotating_5_Star(255-i,buf);
        ScreenBlendUp(buf);
        Do_Rotating_5_Star(255,buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_to_white(0,255);
        UpdateRotation(2,1,3);
        Do_Rotating_5_Star(63,buf);
        ScreenBlendUp(buf);
        Do_Rotating_5_Star(255,buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    ClearScreen(0,buf);
    ClearScreen(0,tmp);
    LeggiPcx("Bump1.Pcx",buf,loadpal);
    LeggiPcx("Bump2.Pcx",tmp2,loadpal);

    for ( i=0; i<3; i++ )
        ScreenBlend(buf);
    for ( i=0; i<3; i++ )
        ScreenBlend(tmp2);

    SetBumpPal1();
    t=t2=0;
    for ( t=0; t<63; t++ )
    {
        dummy=update_fade_in();
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( ; t<256; t++ )
    {
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    SetBumpPal2();
    for ( t=0; t<63; t++ )
    {
        dummy=update_fade_in();
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    for ( ; t<256; t++ )
    {
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    for ( t=0 ; t<256; t++ )
    {
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump_2_Image(tmp,buf,tmp2,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    for ( t=0; t<256; t++ )
    {
        ClearScreen(0,tmp);
        t2=sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    for ( t=0; t<64; t++ )
    {
        dummy=update_fade_to_black(0,255);
        ClearScreen(0,tmp);
        t2=t+sine[t];
        c=160+(sine[(t+kangle90) & kanglemask]>>1);
        r=100+(sine[t2 & kanglemask]>>2);
        Do_Bump(tmp,buf,c,r);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    ClearScreen(0,buf);
    ClearScreen(0,vga);
    SetFirePal1();
    xangle=zangle=yangle=0;
    zoff=-64;
    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_in();
        UpdateRotation(0,1,0);
        Do_Cube(160,100,buf);
        ScreenBlend(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<232; i++ )
    {
        UpdateRotation(0,1,0);
        Do_Cube(160,100,buf);
        ScreenBlend(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    SetFirePal2();
    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_in();
        UpdateRotation(0,1,0);
        Do_Cube(160,100,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }
    for ( i=0; i<192; i++ )
    {
        zoff-=2;
        UpdateRotation(0,1,1);
        Do_Cube(160,100,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    start=clock();
    do {
        UpdateRotation(0,1,1);
        Do_Cube(160,100,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
        elapsed=clock()-start;
    } while (elapsed<10000);

    for ( i=0; i<63; i++ )
    {
        dummy=update_fade_to_black(0,255);
        UpdateRotation(1,1,1);
        Do_Cube(160,100,buf);
        ScreenBlendUp(buf);
        FlipMem(tmp,buf);
        ShowVuMeters(tmp);
        FastTrasp(tmp,trsp,0);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(tmp);
    }

    LeggiPcx("Lens.pcx",tmp,destpal);
    FlipMem(buf,tmp);
    FlipMem(tmp2,tmp);
    BufToVga(buf);
    Fade_In();
    t=0;
    di=dj=1;
    start=clock();
    do
    {
        FlipMem(buf,tmp);

        i=110+((sine[di & kanglemask] * 156) >> 8);
        j=50+((sine[dj & kanglemask] * 96 ) >> 8);

        di+=2;
        dj++;

        Do_Lens(buf,tmp,i,j);

        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
        elapsed=clock()-start;
    } while ( elapsed<15000 );

    for ( t=0; t<63; t++ )
    {
        dummy=update_fade_to_white(0,255);
        FlipMem(buf,tmp);

        i=110+((sine[di & kanglemask] * 156) >> 8);
        j=50+((sine[dj & kanglemask] * 96 ) >> 8);

        di+=2;
        dj++;

        Do_Lens(buf,tmp,i,j);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    Fade_to_white(0,255);

    LeggiPcx("Lens2.pcx",vga,destpal);
    Fade_In();
    start=clock();
    do
    {
        elapsed=clock()-start;
    } while ( elapsed<5000 );
    Fade_To_Black(0,255);

    ClearScreen(0,vga);
    ClearScreen(0,buf);
    ClearScreen(0,tmp);

    Init_Stars_Structure();
    memset(img,0,16384);
    LeggiPcx("Grizly.Pcx",img2,destpal);
    c=0;

    // Show GRIZLY
    for ( i=0; i<63; i++ )
    {
        ClearScreen(0,buf);
        Do_Wind(img,buf,c,0,0);
        Do_Stars(buf);
        dummy=update_fade_in();
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        for ( l=0; l<16384; l++ )
        {
            t=img[l];
            t2=img2[l];
            if ( t>t2 ) img[l]--;
            if ( t<t2 ) img[l]++;
        }
        Do_Wind(img,buf,c,0,i);
        Stampa(buf,"GFX LIB AND DEMO CODE BY",10,i,SMOOTH);
        Stampa(buf,"G R I Z L Y",180,i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        Do_Wind(img,buf,c,0,255);
        Stampa(buf,"GFX LIB AND DEMO CODE BY",10,255,SMOOTH);
        Stampa(buf,"G R I Z L Y",180,255,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        Do_Wind(img,buf,c,0,255);
        Stampa(buf,"GFX LIB AND DEMO CODE BY",10,255-i,SMOOTH);
        Stampa(buf,"G R I Z L Y",180,255-i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    // Show DEDA
    LeggiPcx("Deda.Pcx",img2,destpal);
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        for ( l=0; l<16384; l++ )
        {
            t=img[l];
            t2=img2[l];
            if ( t>t2 ) img[l]--;
            if ( t<t2 ) img[l]++;
        }
        Do_Wind(img,buf,c,i>>2,255);
        Stampa(buf,"DEMO CODE AND SOME 3D ART BY",10,i,SMOOTH);
        Stampa(buf,"D E D A",180,i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        Do_Wind(img,buf,c,64,255);
        Stampa(buf,"DEMO CODE AND SOME 3D ART BY",10,255-i,SMOOTH);
        Stampa(buf,"D E D A",180,255-i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    // Show FEDAX
    LeggiPcx("Fedax.Pcx",img2,destpal);
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        for ( l=0; l<16384; l++ )
        {
            t=img[l];
            t2=img2[l];
            if ( t>t2 ) img[l]--;
            if ( t<t2 ) img[l]++;
        }
        Do_Wind(img,buf,c,64+(i>>2),255);
        Stampa(buf,"3D GRAPHICS BY",10,i,SMOOTH);
        Stampa(buf,"F E D A X",180,i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        Do_Wind(img,buf,c,128,255);
        Stampa(buf,"3D GRAPHICS BY",10,255-i,SMOOTH);
        Stampa(buf,"F E D A X",180,255-i,SMOOTH);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    // EXIT
    memset(img2,0,16384);
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        c++;
        for ( l=0; l<16384; l++ )
        {
            t=img[l];
            t2=img2[l];
            if ( t>t2 ) img[l]--;
            if ( t<t2 ) img[l]++;
        }
        Do_Wind(img,buf,c,128+i,255-i);
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    ClearScreen(255,vga);
    WaitRet();
    WaitRet();
    for ( i=0; i<256; i++ )
    {
        ClearScreen(0,buf);
        Stampa(buf,"''... TO GO WHERE NO MAN",70,i,SMOOTH);
        Stampa(buf,"HAS GONE BEFORE.''",80,i,SMOOTH);
        Stampa(buf,"CPT. KIRK",110,i,SMOOTH);
        Do_Stars(buf);
        BufToVga(buf);
    }

    start=clock();
    do {
        ClearScreen(0,buf);
        Stampa(buf,"''... TO GO WHERE NO MAN",70,255,SMOOTH);
        Stampa(buf,"HAS GONE BEFORE.''",80,255,SMOOTH);
        Stampa(buf,"CPT. KIRK",110,255,SMOOTH);
        Do_Stars(buf);
        BufToVga(buf);
        elapsed=clock()-start;
    } while (elapsed<5000);

    for ( i=0; i<63; i++ )
    {
        ClearScreen(0,buf);
        Stampa(buf,"''... TO GO WHERE NO MAN",70,255,SMOOTH);
        Stampa(buf,"HAS GONE BEFORE.''",80,255,SMOOTH);
        Stampa(buf,"CPT. KIRK",110,255,SMOOTH);
        Do_Stars(buf);
        BufToVga(buf);
        dummy=update_fade_to_black(0,255);
        dummy=midasSD->SetMasterVolume(63-i);
    }

    // Terminate
    if (nosound==0) midasStopModule(module);

    if ( (error = gmpFreeModule(module)) != OK )
        printf("Unable to dealloc module.\n");

    if ( (error = vuClose()) != OK )
        printf("Unable to deinit VuMeters.\n");

    MIDASclose();

    Fine("Ashes has finished.\n");
}

/*-----------------23/08/97 16.51-------------------
 Intro. Messaggi iniziali.
 Buf è un buffer di aiuto grande come la vga
--------------------------------------------------*/
void Show_Intro(void) {
    WORD t,i;

    Init_Stars_Structure();

    ClearScreen(0,vga);
    ClearScreen(0,buf);

    for ( i=0; i<64; i++ )
    {
        destpal[i].r=0;
        destpal[i].g=0;
        destpal[i].b=i/2;

        destpal[64+i].r=i/2;
        destpal[64+i].g=0;
        destpal[64+i].b=32+i/2;

        destpal[128+i].r=32+i/2;
        destpal[128+i].g=i/2;
        destpal[128+i].b=63;

        destpal[192+i].r=63;
        destpal[192+i].g=32+i/2;
        destpal[192+i].b=63;
    }

    for ( i=0; i<64000; i++ )
        buf[i]=random(255);

    BufToVga(buf);
    Fade_In();

    for ( i=0; i<256; i++ )
    {
        Do_Stars(buf);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        ScreenBlend(buf);
        BufToVga(buf);
    }

    for ( i=0; i<255; i++ )
    {
        t=i;
        if ( t>92 ) t=92;
        Stampa(buf,"THE ROLLING PIXELS",100,t,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"THE ROLLING PIXELS",100,i,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=92; i<255; i++ )
    {
        Stampa(buf,"THE ROLLING PIXELS",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"THE ROLLING PIXELS",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    for ( i=255; i>92; i-- )
    {
        Stampa(buf,"ARE PROUD TO PRESENT",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"ARE PROUD TO PRESENT",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<128; i++ )
    {
        Stampa(buf,"ARE PROUD TO PRESENT",100,92,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"ARE PROUD TO PRESENT",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=92; i<255; i++ )
    {
        Stampa(buf,"ARE PROUD TO PRESENT",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"ARE PROUD TO PRESENT",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    for ( i=255; i>92; i-- )
    {
        Stampa(buf,"THEIR FIRST DEMO",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"THEIR FIRST DEMO",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<128; i++ )
    {
        Stampa(buf,"THEIR FIRST DEMO",100,92,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"THEIR FIRST DEMO",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=92; i<255; i++ )
    {
        Stampa(buf,"THEIR FIRST DEMO",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"THEIR FIRST DEMO",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    for ( i=255; i>92; i-- )
    {
        Stampa(buf,"CALLED",100,i,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"CALLED",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }
    for ( i=0; i<128; i++ )
    {
        Stampa(buf,"CALLED",100,92,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf,"CALLED",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
    }

    for ( i=0; i<128; i++ )
    {
        Stampa(buf," ",100,92,FLAT);
        Do_Stars(buf);
        ScreenBlend(buf);
        Stampa(buf," ",100,255,FLAT);
        oldCount=frameCount;
        while ( frameCount==oldCount )
        BufToVga(buf);
        dummy=update_fade_to_white(0,255);
    }
}

/*-----------------23/08/97 16.17-------------------
 Effetto bandiera con light source.
 In img c'è l'immagine che deve essere processata.
 Deve essere di 128*128 pixel e NON viene modificata.
 In where va messa l'area di memoria di destinazione,
 ampia come la vga.
 Angle e Phase sono i valori dello sventolio.
 Modificandoli continuamente in un ciclo si ottiene
 l'effetto bandiera.
--------------------------------------------------*/
void Do_Wind(BYTE *img, BYTE *where, int angle, int phase, int shiny)
{
    int x,y;
    int nx=0,ny=0,sy=0;
    int onx=0;
    int dx=0;
    int i=0,t,fx,fy;
    BYTE tx,ty;

    onx=(sine[(angle+angle+phase) & kanglemask]>>4);

    for ( y=0; y<128; y++ )
    {
        sy=y<<7;
        tx=angle+angle+y+phase;
        ty=angle+kangle90;

        for ( x=0; x<128; x++ ) {
            i=img[sy];
            tx++;
            ty++;
            fx=(sine[tx]>>4);
            fy=(sine[ty]>>4);

            nx=x+fx;
            ny=y+fy;
            ny=(ny<<6)+(ny<<8);

            t=i+fx+fy+fx+fy;
            for (dx=onx; dx<=nx; dx++)
                where[11612+ny+dx]=t < 0 ? 0 : t > shiny ? shiny : t;

           onx=nx;
           sy++;
        }
    }
}

/*-----------------23/08/97 16.14-------------------
 Disegna l'effeto Starfield nel buffer passato, che
 deve essere grande come la vga, e aggiorna la
 posizione delle stelle nella rispettiva struttura,
 in modo che alla chiamata successiva le stelle
 avanzino verso gli occhi dell'utente.

 All'inizio del programma va chiamata la procedura
 Init_ che inizializza la struttura delle stelle.
--------------------------------------------------*/
void Init_Stars_Structure(void)
{
    int i;
    for (i=0;i<NUMSTARS;i++) {
        stars[i].x=random(320)-160;
        stars[i].y=random(200)-100;
        stars[i].z=(random(512)+1);
    }

}

void Do_Stars(BYTE *where)
{
    WORD tx,ty,i;
    int c,p;

    for (i=0;i<NUMSTARS;i++) {
        tx=160+((stars[i].x<<8) / stars[i].z) ;
        ty=100+((stars[i].y<<8) / stars[i].z) ;

        stars[i].z-=2;

        if (tx>=320 || tx==0 || ty>=200 || ty==0 || stars[i].z<2) {
            stars[i].x=random(320)-160;
            stars[i].y=random(200)-100;
            stars[i].z=512;
            tx=160+(stars[i].x<<8)/stars[i].z;
            ty=100+(stars[i].y<<8)/stars[i].z;
        }

        p=tx+((ty<<8)+(ty<<6));
        c=where[p];
        c+=256-(stars[i].z>>1);
        where[p]=c > 255 ? 255 : c;
    }
}

/*-----------------23/08/97 16.12-------------------
 Produce l'effetto fiamme nel buffer passato.
 Il buffer deve essere grande come lo schermo vga.
 L'effetto e' interattivo.
--------------------------------------------------*/
void Do_Flame(BYTE *where, BYTE min, BYTE max)
{
    register WORD i;
    register int temp;

    for(i=0;i<35;i++) {
        temp=20+random(280)+63360;
        where[temp] = min+random(max);
    }

    Render_Flame(where);

    for (i=63360; i<63680; i++)
        if (where[i]>1) {
            where[i]--;
            where[i-320]=where[i];
        }

    for (i=63360; i<63680; i++)
        if (random(100)<50) where[i]=(where[i]+where[i+1])>>1;
        else where[i]=(where[i]+where[i-1])>>1;
}


/*-----------------24/08/97 11.17-------------------
 Inizializza la struttura che contiene i vertici
 di una stella a 5 punte.
 La procedura successiva disegna la stella rotante
 nella regione where.
--------------------------------------------------*/
void Init_Penta_Structure(void) {
    int t=0, l=0;
    for ( l=0; l<5; l++ )
    {
        stella[l].x=(sine[(t+kangle90) & kanglemask]/4);
        stella[l].y=(sine[(t) & kanglemask]/4);
        stella[l].z=(sine[(t) & kanglemask]/8);
        t+=102;
    }
}

void Do_Rotating_5_Star(BYTE col, BYTE *where)
{
    int t;
    int x1,y1,x2,y2,x3,y3,x4,y4,x5,y5;

    // Ruota punto 1
    x=stella[0].x;
    y=stella[0].y;
    z=stella[0].z;
    RotatePoint();

    t=(z-zoff);
    x1=((x<<8)/t)+160;
    y1=((y<<8)/t)+100;

    // Ruota punto 2
    x=stella[1].x;
    y=stella[1].y;
    z=stella[1].z;
    RotatePoint();

    t=(z-zoff);
    x2=((x<<8)/t)+160;
    y2=((y<<8)/t)+100;

    // Ruota punto 3
    x=stella[2].x;
    y=stella[2].y;
    z=stella[2].z;
    RotatePoint();

    t=(z-zoff);
    x3=((x<<8)/t)+160;
    y3=((y<<8)/t)+100;

    // Ruota punto 4
    x=stella[3].x;
    y=stella[3].y;
    z=stella[3].z;
    RotatePoint();

    t=(z-zoff);
    x4=((x<<8)/t)+160;
    y4=((y<<8)/t)+100;

    // Ruota punto 5
    x=stella[4].x;
    y=stella[4].y;
    z=stella[4].z;
    RotatePoint();

    t=(z-zoff);
    x5=((x<<8)/t)+160;
    y5=((y<<8)/t)+100;

    Bline(x1,y1,x2,y2,col,where);
    Bline(x2,y2,x3,y3,col,where);
    Bline(x3,y3,x4,y4,col,where);
    Bline(x4,y4,x5,y5,col,where);
    Bline(x5,y5,x1,y1,col,where);
}


/*-----------------26/08/97 15.34-------------------
 Effettua il BUMP Mapping nella zona di memoria
 DEST, leggendo i dati in SRC,
 che devono essere grandi come la vga.
 lx e ly sono le coordinate della luce.
 La palette in uso deve contenere i valori da 0 a
 127 posti a 0, da 128 in poi un colore sfumato.
--------------------------------------------------*/
void Do_Bump(BYTE *dest, BYTE *src, int lx, int ly)
{
    int vxl,vyl,nx,ny,difx,dify,col;
    int video=320;
    int yb,xb;

    for ( yb=1; yb<199; yb++ )
    {
        video++;
        vyl=yb-ly;

        for ( xb=1; xb<319; xb++ )
        {
            vxl=xb-lx;

            nx=src[video+1]-src[video-1];
            ny=src[video+320]-src[video-320];

            col=vxl-nx;
            if ( col<0 ) col=-col;
            difx=127-col;

            col=vyl-ny;
            if ( col<0 ) col=-col;
            dify=127-col;

            difx+=dify;
            if (difx>0) dest[video]=difx;
            video++;
        }
        video++;
    }
}

void Do_Bump_2_Image(BYTE *dest, BYTE *src, BYTE *src2, int lx, int ly)
{
    int vxl,vyl,nx,ny,difx,dify,col;
    int video=320;
    int yb,xb;
    BYTE s1,s2;

    for ( yb=1; yb<199; yb++ )
    {
        video++;
        vyl=yb-ly;

        for ( xb=1; xb<319; xb++ )
        {
            s1=src[video];
            s2=src2[video];
            if ( s1>s2 ) src[video]--;
            if ( s1<s2 ) src[video]++;

            vxl=xb-lx;
            nx=src[video+1]-src[video-1];
            ny=src[video+320]-src[video-320];

            col=vxl-nx;
            if ( col<0 ) col=-col;
            difx=127-col;

            col=vyl-ny;
            if ( col<0 ) col=-col;
            dify=127-col;

            difx+=dify;
            if (difx>0) dest[video]=difx;

            video++;
        }
        video++;
    }
}

/*-----------------31/08/97 15.54-------------------
 La prima funzione inizializza la struttura del
 cubo, la seconda disegna il cubo il where.
--------------------------------------------------*/
void Init_Cube_Structure(void)
{
    int cx,cy,cz,i;
    i=0;
    for (cz=0; cz<8; cz++) {
        for (cy=0; cy<8; cy++) {
            for (cx=0; cx<8; cx++) {
                stars[i].x=5+(cx-4)*10;
                stars[i].y=5+(cy-4)*10;
                stars[i].z=5+(cz-4)*10;
                i++;
            }
        }
    }
}

void Do_Cube(int cx, int cy, BYTE *where)
{
    int l,c,r,t;

    for ( l=0; l<NUMPOINTS; l++ )
    {
        x=stars[l].x;
        y=stars[l].y;
        z=stars[l].z;
        RotatePoint();

        t=(z-zoff);
        c=((x<<8)/t)+cx;
        r=((y<<8)/t)+cy;

        if ( (c>0) && (c<319) && (r>0) && (r<199) )
            where[c+(r<<6)+(r<<8)]=x+y-z+126;
    }
}

int CALLING MakeMeter(sdSample *sdsmp, gmpSample *gmpsmp)
{
    return vuPrepare(sdsmp, gmpsmp->sdHandle - 1);
}

void ShowVuMeters(BYTE *where)
{
    int i=0;

    for ( i = 0; i < numChannels; i++ )
    {
        midasSD->GetRate(i, &rate);
        midasSD->GetPosition(i, &position);
        midasSD->GetSample(i, &sample);
        midasSD->GetVolume(i, &volume);
        if ( (sample > 0) && (rate > 0) )
            vuMeter(sample-1, rate, position, volume, &meter);

        Hline(0,meter,(i<<3)+160,96+meter,where);
        Hline(0,meter,(i<<3)+161,96+meter,where);
        Hline(0,meter,(i<<3)+162,96+meter,where);
        Hline(0,meter,(i<<3)+163,96+meter,where);
    }
}

void Build_Lens_Trasf_Matx(int d, int m)
{
    int r,s,a,b,z;
    BYTE *ctmp;

    // Leggi la mappa della luce
    ctmp=malloc(d*d);
    if ( ctmp==NULL ) Fine("Unable to allocate *ctmp.\n");
    printf("þ");

    LeggiPcx("light.pcx",ctmp,destpal);
    printf("þ");

    // Raggio
    r=d/2;

    // Circonferenza data dall'intersezione di un piano distante
    // "m" dal centro della sfera e la sfera stessa.
    s=sqrt(r*r-m*m);

    // Crea la matrice per la trasformazione
    // Controlla se i punti sono all'esterno o all'interno di s
    for ( y=-r; y<-r+d; y++ )
    {
        for ( x=-r; x<-r+d; x++ )
        {
            if ( (x*x)+(y*y)>=(s*s) )
            {
                // All'esterno: non cambia niente
                // Il punto e' trasparente.
                a=x;
                b=y;
                z=0;
                *(mtx+((y+r)*d+(x+r)))=-1;
            } else {
                // All'interno: il punto viene proiettato sulla sfera.
                z=sqrt(r*r-x*x-y*y);
                a=x*m/z;
                b=y*m/z;
                // Se il punto sul piano ha coordinate x,y sulla sfera
                // avra' coordinate a,b
                *(mtx+((y+r)*d+(x+r)))=((b+r)*scrx+(a+r));
            }
        }
    }
    printf("þ");

    // Crea la matrice per la mappatura della luce
    for ( y=-r; y<-r+d; y++ )
    {
        for ( x=-r; x<-r+d; x++ )
        {
            if ( (x*x)+(y*y)>=(s*s) )
            {
                a=x;
                b=y;
                z=0;
                *(colmtx+((y+r)*d+(x+r)))=0;
            } else {
                z=sqrt(r*r-x*x-y*y);
                a=x*m/z;
                b=y*m/z;
                *(colmtx+((y+r)*d+(x+r)))=*(ctmp+((b+r)*d+(a+r)));
            }
        }
    }
    printf("þ");

    free(ctmp);
    printf("þ Ok.\n");
}

void Do_Lens(BYTE *dest, BYTE *src, int lx, int ly)
{
    int t,t2,x,y,offset1,offset2;

    offset1=lx+(ly<<8)+(ly<<6);

    for ( y=0; y<DIMENSIONE; y++)
    {
        offset2=(y<<8)+(y<<6);
        for ( x=0; x<DIMENSIONE; x++ )
        {
            if (    ((lx+x) > 0) &&
                    ((lx+x) < scrx) &&
                    ((ly+y) > 0) &&
                    ((ly+y) < scry) )
            {
                /* Trasformata del pixel corrente.
                   in t ho il valore dell'offset del pixel che devo leggere
                   nell'immagine originale per ottenere l'effetto
                   lente nella destionazione */
                t=*(mtx+(x+y*DIMENSIONE));

                /* Leggo la mappa del colore associata a
                   questo pixel */
                t2=*(colmtx+(x+y*DIMENSIONE));

                // Calcolo il colore del pixel
                t2=((src[offset1+t]+t2)>>1);
                //if ( t2>255 ) t2=255;
                if ( t>0 ) dest[offset1+x+offset2]=t2;
            }
        }
    }
}

void Fine(char *msg)
{
    VideoMode(0x03);

    if ( buf != NULL )      free(buf);
    if ( tmp != NULL )      free(tmp);
    if ( tmp2 != NULL )     free(tmp2);
    if ( img != NULL )      free(img);
    if ( img2 != NULL )     free(img2);
    if ( loadpal != NULL)   free(loadpal);
    if ( mtx != NULL )      free(mtx);
    if ( colmtx != NULL )   free(colmtx);
    if ( trsp != NULL)      free(trsp);

    printf(msg);
    getch();
    exit(0);
}