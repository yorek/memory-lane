//
// Free Drictional Starfield
// FIRST ATTEMP
//
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <i86.h>
#include "..\bye.h"

#define BENCH
#ifdef BENCH
    #include <time.h>
#endif

// Vga
#define vga 0x0A0000
#define VGA_STATUS 0x3DA
#define VGA_VSYNC_MASK 8

// Random MACRO
#define RANDOM(n)   (((float)rand() / (float)RAND_MAX)*(n))

// Number of stars
#define MAXSTARS 4000

// Angles
#define ANGLES 2048
#define ANGLEMASK ANGLES-1

//
// Star structure
//
typedef struct
{
    float x,y,z;
    float cx,cy,cz;
    float vz,vx,vy;
} STAR;

unsigned char *buffer   =NULL;
STAR *starfield         =NULL;
unsigned short int *yval=NULL;
float *sine             =NULL;
float *cosine           =NULL;
char fix;

void StarField(void);
void SetColors(void);
int InitMem(void);
void FreeMem(void);
void WaitRet(void);
void CalculateStarPosition(int i, int ax, int ay, int az);
void CheckAndDraw(int i);

int main(int argc, char *argv[])
{
    union REGS r;
    struct SREGS sr;
    char *systems[] = {"Raw", "XMS", "VCPI", "DPMI"};
    char *processor[] = {"80386", "80486", "80586","Unknown"};

    segread (&sr);
    r.w.ax = 0xEEFF;
    int386x (0x31, &r, &r, &sr);

    printf("\nFree Directional Starfield v1.0 (c)1998 Manowar / Rolling Pixels\n\n");

    if (r.x.cflag) {
        printf ("Extender Info Function Not Present!\n");
        exit (1);
    }

    printf ("System Type        : %s\n", systems[r.h.ch]);

    if (r.h.cl > 5) r.h.cl = 6;
    printf ("Processor          : %s (Recommended: 586) \n\n", processor[r.h.cl-3]);

    fix=0;
    if ( argc>1 )
    {
        if ( argv[1][0]=='-' || argv[1][0]=='/')
        {
            if ( argv[1][1]=='s' || argv[1][1]=='S')
            fix=1;
        }
    }
    StarField();

    return(1);
}

//
// Main
//
void StarField(void)
{
    union REGS rg;
    int i;
    int ax,ay,az;
    #ifdef BENCH
        int start,end;
        int frames;
    #endif

    printf("Initializing...\n\n");

    //
    // Allocate memory
    //
    i=InitMem();
    if ( i==0 )
    {
        printf("[InitMem] : Error allocating memory\n");
        FreeMem();
        exit(1);
    }
    printf("[InitMem] : Allocated %ld kb\n",(i>>10));

    //
    // Fill offset lookup table
    //
    for ( i=0; i<200; i++ )
    {
        yval[i]=i*320;
    }
    printf("[StarField] : offset lookup created\n");

    //
    // Fill trigonometric tables
    //
    for ( i=0; i<ANGLES; i++ )
    {
        sine[i]=sin((float)i * 3.1415 / (ANGLES/2));
        cosine[i]=cos((float)i * 3.1415 / (ANGLES/2));
    }
    printf("[StarField] : %ld angles lookup created (2 tables)\n", ANGLES);

    //
    // Fill starfiled structure
    //
    for ( i=0; i<MAXSTARS; i++ )
    {
        starfield[i].x=(RANDOM(2)-1)*256;
        starfield[i].y=(RANDOM(2)-1)*256;
        starfield[i].z=(RANDOM(2)-1)*256;

        if ( fix==0 ) starfield[i].vz=-RANDOM(3);
        else starfield[i].vz=0;
    }
    printf("[StarField] : starfield filled with %ld stars\n", MAXSTARS);
    if ( fix==1 ) printf("[StarField] : starfield will stand still (-s option)\n");

    //
    // Reset counter if BENCHMARKING
    //
    #ifdef BENCH
        start=end=0;
        frames=0;
        printf("[StarField] : (DEBUG) Timer and counter initialized\n");
        getch();
    #endif

    // Set gfx mode
    rg.w.ax=0x13;
    int386(0x10,&rg,&rg);

    // Set Colors
    SetColors();

    // Start!
    memset(buffer,0,64000);
    ax=ay=az=0;
    #ifdef BENCH
        start=clock();
    #endif
    do
    {
        memset(buffer,0,64000);

        for ( i=0; i<MAXSTARS; i++ )
        {
            // Calculate the new star position due to the camera angle
            CalculateStarPosition(i,ax,ay,az);

            // If star is visible draw it
            CheckAndDraw(i);

            // Move star towards the observer's eye
            starfield[i].z=starfield[i].z+starfield[i].vz;

            // If star out of starfield make a new star
            if ( starfield[i].z<-256 )
            {
                starfield[i].x=(RANDOM(512)-256);
                starfield[i].y=(RANDOM(512)-256);
                starfield[i].z=256;
            }

        }
        az++;
        az&=ANGLEMASK;
        ay--;
        ay&=ANGLEMASK;
        ax+=2;
        ax&=ANGLEMASK;

        #ifdef BENCH
            frames++;
        #else
            WaitRet();
        #endif

        memcpy(vga, buffer, 64000);
    } while (kbhit()==0);
    #ifdef BENCH
        end=clock();
    #endif
    getch();

    rg.w.ax=0x03;
    int386(0x10,&rg,&rg);

    FreeMem();
    #ifdef BENCH
        printf("[StarField] : (DEBUG) Time elapsed     : %f\n",(float)(end-start)/1000.0);
        printf("[StarField] : (DEBUG) Frames displayed : %ld\n",frames);
        printf("[StarField] : (DEBUG) Fps              : %f\n",(float)frames/((float)(end-start)/1000.0));
    #else
        Bye();
    #endif
}

//
// Alloca memoria
//
int InitMem(void)
{
    int temp,tot;

    temp=320*200;
    buffer=malloc(temp);
    if (buffer==NULL) return 0;
    tot=temp;

    temp=200*sizeof(unsigned short int);
    yval=malloc(temp);
    if (yval==NULL) return 0;
    tot+=temp;

    temp=ANGLES*sizeof(float);
    sine=malloc(temp);
    if (sine==NULL) return 0;
    tot+=temp;

    temp=ANGLES*sizeof(float);
    cosine=malloc(temp);
    if (cosine==NULL) return 0;
    tot+=temp;

    temp=MAXSTARS*sizeof(STAR);
    starfield=malloc(temp);
    if (starfield==NULL) return 0;
    tot+=temp;

    return tot;
}

//
// Libera memoria
//
void FreeMem(void)
{
    if ( buffer!=NULL ) free(buffer);
    if ( starfield!=NULL ) free(starfield);
    if ( yval!=NULL ) free(yval);
    if ( sine!=NULL ) free(sine);
    if ( cosine!=NULL ) free(cosine);
}

//
// Aspetta il VR
//
void WaitRet(void)
{
    while(inp(VGA_STATUS) & VGA_VSYNC_MASK);        //wait for end of current retrace
    while( !(inp(VGA_STATUS) & VGA_VSYNC_MASK));    //wait for end of drawing period
}

//
// Setta la palette
//
void SetColors(void)
{
    int i;
    for ( i=0; i<63; i++ )
    {
        outp(0x3C8,i);
        outp(0x3C9,i);
        outp(0x3C9,i);
        outp(0x3C9,i);
    }
}

//
// Calcola la posizione della stella in accordo con l'angolazione
// della telecamera.
//
void CalculateStarPosition(int i, int ax, int ay, int az)
{
    float tx,ty,tz;
    STAR *temp;

    temp=&starfield[i];

    temp->cx=temp->x;
    temp->cy=temp->y;
    temp->cz=temp->z;

    // Rotate camera around x
    ty=temp->cy*cosine[ax] - temp->cz*sine[ax];
    tz=temp->cy*sine[ax] + temp->cz*cosine[ax];
    temp->cy=ty;
    temp->cz=tz;

    // Rotate camera around y
    tx=temp->cx*cosine[ay] + temp->cz*sine[ay];
    tz=-(temp->cx*sine[ay]) + temp->cz*cosine[ay];
    temp->cx=tx;
    temp->cz=tz;

    // Rotate camera around z
    tx=temp->cx*cosine[az] - temp->cy*sine[az];
    ty=temp->cx*sine[az] + temp->cy*cosine[az];
    temp->cx=tx;
    temp->cy=ty;
}

//
// Se la stella e' visibile la disegna
//
void CheckAndDraw(int i)
{
    int x,y,j;
    STAR *temp;

    temp=&starfield[i];

    if ( temp->cz>0 )
    {
        x=160+(temp->cx*160)/temp->cz;
        y=100+(temp->cy*100)/temp->cz;

        j=64-temp->cz/4;
        if ( x>-1 && x<320 && y>-1 && y<200 )
            if (buffer[x+yval[y]]<j) buffer[x+yval[y]]=j;
    }
}