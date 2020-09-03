//
// Free Drictional Starfield
//
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <i86.h>
#include "write.h"
#include "colors.h"
#include "..\bye.h"

//#define SHOW
//#define BENCH
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
#define MAXSTARS 2500

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
    float vz;
} STAR;

unsigned char *buffer   =NULL;
STAR *starfield         =NULL;
unsigned short int *yval=NULL;
float *sine             =NULL;
float *cosine           =NULL;
float *random           =NULL;
char fix,mode;

void StarField(void);
void SetColors(void);
int InitMem(void);
void FreeMem(void);
void WaitRet(void);
void CalculateStarPosition(int i, int ax, int ay, int az);
void CalculateAllStars(int ax, int ay, int az);
void CheckAndDraw(int i);

int main(int argc, char *argv[])
{
    union REGS r;
    struct SREGS sr;
    char *systems[] = {"Raw", "XMS", "VCPI", "DPMI"};
    char *processor[] = {"80386", "80486", "80586","Unknown"};
    int i;

    segread (&sr);
    r.w.ax = 0xEEFF;
    int386x (0x31, &r, &r, &sr);

    printf("\nFree Directional Starfield v1.0 (c)1998 Manowar / Rolling Pixels\n");
    printf("Use /? for options.\n\n");

    if (r.x.cflag) {
        printf ("Extender Info Function Not Present!\n");
        exit (1);
    }
    printf ("System Type        : %s\n", systems[r.h.ch]);
    if (r.h.cl > 5) r.h.cl = 6;
    printf ("Processor          : %s (Recommended: 586) \n\n", processor[r.h.cl-3]);

    fix=0;
    mode=0;
    if ( argc>1 )
    {
        for ( i=1; i<argc; i++ )
        {
            if ( argv[i][0]=='-' || argv[i][0]=='/')
            {
                if ( argv[i][1]=='s' || argv[i][1]=='S')
                    fix=1;
            }

            if ( argv[i][0]=='-' || argv[i][0]=='/')
            {
                if ( argv[i][1]=='a' || argv[i][1]=='A')
                    mode=1;
            }

            if ( argv[i][0]=='-' || argv[i][0]=='/')
            {
                if ( argv[i][1]=='?' )
                {
                    printf(" /s     Don't fly trough starfield, only move camera.\n");
                    printf(" /a     Make starfield as if we are in a planet's ring.\n\n");
                    exit(0);
                }
            }
        }
    }

    printf("--- KEYS ---\n");
    printf("x/X = add/subtract one to angular X velocity\n");
    printf("y/Y = add/subtract one to angular Y velocity\n");
    printf("z/Z = add/subtract one to angular Z velocity\n");
    printf("q   = quit\n\n");
    StarField();

    return(1);
}

//
// Main
//
void StarField(void)
{
    union REGS rg;
    int i,quit;
    int ax,ay,az;
    int vx,vy,vz;

    #ifdef SHOW
        char mex[80],mex2[80];
    #endif

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
    // Fill random lookup table
    //
    for ( i=0; i<MAXSTARS; i++ )
    {
        random[i]=(RANDOM(2)-1)*256;
        if ( mode==0 ) random[MAXSTARS+i]=(RANDOM(2)-1)*255;
        else random[MAXSTARS+i]=(RANDOM(2)-1)*32;
    }
    if (mode==1) printf("[StarField] : planet's ring mode on\n");
    printf("[StarField] : random lookup table filled with %ld values\n", MAXSTARS*2);

    //
    // Fill starfiled structure
    //
    for ( i=0; i<MAXSTARS; i++ )
    {
        starfield[i].x=random[i];
        starfield[i].y=random[MAXSTARS+i];
        starfield[i].z=(RANDOM(2)-1)*256;

        if ( fix==0 ) starfield[i].vz=-RANDOM(3);
        else starfield[i].vz=0;
    }
    printf("[StarField] : starfield filled with %ld stars\n", MAXSTARS);
    if ( fix==1 ) printf("[StarField] : starfield will stand still (-s option)\n");

    #ifdef SHOW
        printf("[StarField] : Show on\n");
    #endif

    //
    // Reset counter if BENCHMARKING
    //
    #ifdef BENCH
        start=end=0;
        frames=0;
        printf("[StarField] : (DEBUG) timer and counter initialized\n");
        printf("[StarField] : (DEBUG) starfield won't wait for VR\n");
    #endif
    getch();

    // Set gfx mode
    rg.w.ax=0x13;
    int386(0x10,&rg,&rg);

    // Set Colors
    SetColors();

    // Start!
    memset(buffer,0,64000);
    ax=ay=az=0;
    vx=vy=vz=0;
    quit=0;

    #ifdef SHOW
        sprintf(mex,"aX: %4d - aY: %4d - aZ: %4d", ax, ay, az);
        sprintf(mex2,"vX: %4d - vY: %4d - vZ: %4d", vx, vy, vz);
    #endif
    #ifdef BENCH
        start=clock();
    #endif
    do
    {
        do
        {
            memset(buffer,0,64000);

            // Calculate the new star position due to the camera angle
            // done outaside the loop below in order to maxize optimization
            // during the rotation. (Used 9 muls per cycle insted of 12)
            CalculateAllStars(ax,ay,az);

            for ( i=0; i<MAXSTARS; i++ )
            {
                // If star is visible draw it
                CheckAndDraw(i);

                // Move star towards the observer's eye
                starfield[i].z=starfield[i].z+starfield[i].vz;

                // If star out of starfield make a new star
                if ( starfield[i].z<-256 )
                {
                    starfield[i].x=random[i];
                    starfield[i].y=random[MAXSTARS+i];
                    starfield[i].z=256;
                }

            }

            az+=vz; az&=ANGLEMASK;
            ay+=vy; ay&=ANGLEMASK;
            ax+=vx; ax&=ANGLEMASK;

            #ifdef BENCH
                frames++;
            #else
                WaitRet();
            #endif

            #ifdef SHOW
                sprintf(mex,"aX: %4d - aY: %4d - aZ: %4d", ax, ay, az);
                Write4(buffer,mex,150,10,255);
                Write4(buffer,mex2,150,20,255);
            #endif

            memcpy(vga, buffer, 64000);
        } while (kbhit()==0);

        // state machine
        switch ( getch() )
        {
            case 'x': vx++; break;
            case 'X': vx--; break;
            case 'y': vy++; break;
            case 'Y': vy--; break;
            case 'z': vz++; break;
            case 'Z': vz--; break;
            case 'q': quit=1;
        }
        #ifdef SHOW
            sprintf(mex2,"vX: %4d - vY: %4d - vZ: %4d", vx, vy, vz);
        #endif
    } while ( quit==0 );
    #ifdef BENCH
        end=clock();
    #endif

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

    temp=MAXSTARS*2*sizeof(float);
    random=malloc(temp);
    if (random==NULL) return 0;
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
    if ( random!=NULL ) free(random);
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
    outp(0x3C8,0);
    for ( i=0; i<768; i++ )
        outp(0x3C9,(255-color[i])>>2);
}

//
// Calcola la posizione della stella in accordo con l'angolazione
// della telecamera.
//
void CalculateAllStars(int ax, int ay, int az)
{
    float a,a2,b,b2,d,d2,e,e2,f,g,h,i;
    int j;
    STAR *temp;

    temp=starfield;

    /*

    // The bare rotation algorithm
    tx=(x*c2 + (y*s1 + z*c1)*s2)*c3 - (y*c1 - z*s1)*s3;
    ty=(x*c2 + (y*s1 + z*c1)*s2)*s3 + (y*c1 - z*s1)*c3;
    tz=-(x*s2) + (y*s1 + z*c1)*c2;

    // Expanded
    tx=x*c2*c3 + y*s1*s2*c3 + z*c1*s2*c3 - y*c1*s3 + z*s1*s3;
    ty=x*c2*s3 + y*s1*s2*s3 + z*c1*s2*s3 + y*c1*c3 - z*s1*c3;
    tz=-x*s2 + y*s1*c2 + z*c1*c2;

    // Grouped by axis
    // From this you can made this 9muls/cicle rotation
    tx=x(c2*c3) + y(s1*s2*c3 - c1*s3) + z(c1*s2*c3 + s1*s3);
    ty=x(c2*s3) + y(s1*s2*s3 + c1*c3) + z(c1*s2*s3 - s1*c3);
    tz=x(-s2) + y(s1*c2) + z(c1*c2);

    */

    a=sine[ax]*cosine[az];  a2=a*sine[ay];
    b=sine[ax]*sine[az];    b2=b*sine[ay];
    d=cosine[ax]*sine[az];  d2=d*sine[ay];
    e=cosine[ax]*cosine[az]; e2=e*sine[ay];
    f=cosine[ay]*cosine[az];
    g=cosine[ay]*sine[az];
    h=sine[ax]*cosine[ay];
    i=cosine[ax]*cosine[ay];

    for ( j=0; j<MAXSTARS; j++ )
    {
        temp->cx=temp->x*(f)        + temp->y*(a2 - d)    + temp->z*(e2 + b);
        temp->cy=temp->x*(g)        + temp->y*(b2 + e)    + temp->z*(d2 - a);
        temp->cz=temp->x*(-sine[ay])+ temp->y*(h)         + temp->z*(i);

        temp++;
    }
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

        j=255-temp->cz;
        if ( x>-1 && x<320 && y>-1 && y<200 )
            if (buffer[x+yval[y]]<j) buffer[x+yval[y]]=j;
    }
}