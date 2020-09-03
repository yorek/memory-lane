//
// FireWorks! (c) 1998 by Manowar / Rolling Pixels
//
// Thanx to Tom Hammersley for its doc on particle systems.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <i86.h>

//#define SHOWFPS

// Vga
#define vga 0x0A0000

// Random MACRO
#define RANDOM(n)   (((float)rand() / (float)RAND_MAX)*(n))

// Number of particles for each firework
#define NUMPART 1600

//
// Particle structure: Position, speed
//
typedef struct {
    float   x;
    float   y;
    float   z;
    float   dirx;
    float   diry;
    float   dirz;
} PARTICLE;

//
// Wait for vertical retrace
//
void WaitRet(void);
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

void Bye(void);
void SetColors(void);
void FireWorks(char mode);
void InitExplosion(PARTICLE *particles, int force);
int DrawExplosion(int x, int y, PARTICLE *particles, float d, int c);

unsigned char *buffer;
unsigned short int *yval;
PARTICLE *particlesA;
PARTICLE *particlesB;
PARTICLE *particlesC;
float gravity=0.1;

//
// Main
//

int main(int argc, char *argv[])
{
    printf ("\n\n---    FireWorks   ---\n");
    printf ("PRESS ANY KEY TO START\n\n\n");
    getch();
    FireWorks(0);
    return(1);
}

//
// Main loop
//
void FireWorks(char mode)
{
    union REGS rg;
    clock_t start,end;
    int i,frames;
    int xA,yA,dA;
    int xB,yB,dB;
    int xC,yC,dC;

    //
    // allocate memory
    //
    buffer=malloc(320*200);
    yval=malloc(200*sizeof(unsigned short int));
    particlesA=malloc(NUMPART*sizeof(PARTICLE));
    particlesB=malloc(NUMPART*sizeof(PARTICLE));
    particlesC=malloc(NUMPART*sizeof(PARTICLE));

    if ( buffer==NULL || yval==NULL || particlesA==NULL || particlesB==NULL || particlesC==NULL)
    {
        printf("* Unable to allocate bare memory for demo. BYE!\n");
        exit(1);
    }

    //
    // Calculate lookup offset table
    //
    for ( i=0; i<200; i++ )
    {
        yval[i]=i*320;
    }

    //
    // Go to graph mode 320x200x256
    //
    rg.w.ax=0x13;
    int386(0x10,&rg,&rg);
    for ( i=1; i<70; i++ )
        WaitRet();

    //
    // Set palette
    //
    SetColors();

    //
    // Clear Screen
    //
    memset(buffer,0,64000);

    //
    // Go!
    //
    InitExplosion(particlesA,2);
    xA=RANDOM(320);
    yA=RANDOM(200);
    dA=1024+(RANDOM(2)-1)*256;

    InitExplosion(particlesB,4);
    xB=RANDOM(320);
    yB=RANDOM(200);
    dB=1024+(RANDOM(2)-1)*256;

    InitExplosion(particlesC,6);
    xC=RANDOM(320);
    yC=RANDOM(200);
    dC=1024+(RANDOM(2)-1)*256;

    frames=0;
    memset(buffer,255,64000);
    start=clock();
    do
    {
        if (DrawExplosion(xA,yA,particlesA,dA,48)==0)
        {
            memset(buffer,255,64000);
            InitExplosion(particlesA,RANDOM(3)+3);
            xA=RANDOM(320);
            yA=RANDOM(200);
            dA=1024+((RANDOM(2)-1)*256);
        }
        if (DrawExplosion(xB,yB,particlesB,dB,0)==0)
        {
            memset(buffer,255,64000);
            InitExplosion(particlesB,RANDOM(3)+3);
            xB=RANDOM(320);
            yB=RANDOM(200);
            dB=1024+((RANDOM(2)-1)*256);
        }
        if (DrawExplosion(xC,yC,particlesC,dC,-48)==0)
        {
            memset(buffer,255,64000);
            InitExplosion(particlesC,RANDOM(3)+3);
            xC=RANDOM(320);
            yC=RANDOM(200);
            dC=1024+((RANDOM(2)-1)*256);
        }

        #ifndef SHOWFPS
            WaitRet();
        #endif

        memcpy(vga,buffer,64000);
        memset(buffer,0,64000);

        frames++;
    } while ( !kbhit() );
    end=clock();
    getch();

    //
    // Back to text mode
    //
    rg.w.ax=0x03;
    int386(0x10,&rg,&rg);


    //
    // Show some datas
    //
    #ifdef SHOWFPS
        printf("Frames : %ld\n",frames);
        printf("Time   : %3.3f secs\n",(float)(end-start)/1000.0);
        printf("FPS    : %3.3f\n",(float)(frames*1000.0/(end-start)));
    #endif

    //
    // Final message
    //
    Bye();

    //
    // Free allocated memory
    //
    free(buffer);
    free(yval);
    free(particlesA);
    free(particlesB);
    free(particlesC);
}


//
// Functions
//

void Bye(void)
{
    printf("\n\nFireWorks! (c) 1998 by Manowar of Rolling Pixels.\n");
    printf("Visit our page at:\n");
    printf("   www.geocities.com\\SiliconValley\\Way\\2147  \n");
    printf("\nIf you want contact us please send an e-mail to:\n");
    printf("   manowar-rps@usa.net \n");
    printf("   deda-rps@usa.net \n");
    printf("\nor, if you are on FidoNet to:\n");
    printf("   2:331\\303.17 (MANOWAR) \n");
    printf("\nor contact one of thiz BBS:\n");
    printf("   LANDOVER    : 39-2-6122203 \n");
    printf("   USS STATION : 39-2-39003978 \n");
    printf("and leave a message to Manowar.\n\n");
}

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

void InitExplosion(PARTICLE *particles, int force)
{
    int i;
    float len,lx,ly,lz,dist;

    for ( i=0; i<NUMPART; i++ )
    {
        // Initialize position
        particles[i].x = 0;
        particles[i].y = 0;
        particles[i].z = 1024;

        // Initialize directon vector
        particles[i].dirx = RANDOM(3) - 1.5;
        particles[i].diry = RANDOM(3) - 1.5;
        particles[i].dirz = RANDOM(3) - 1.5;

        // Caculate and normalize vector's lenght
        lx = particles[i].dirx;
        ly = particles[i].diry;
        lz = particles[i].dirz;
        len = sqrt(lx*lx + ly*ly + lz*lz);
        if ( len == 0.0 )
        {
            len = 0.0;
        } else {
            len = 1.0 / len;
        }

        // Now set the speed. This make the explosion to be fitted into a sphere
        dist = RANDOM(force);
        particles[i].dirx *= len*dist;
        particles[i].diry *= len*dist;
        particles[i].dirz *= len*dist;

    }
}

int DrawExplosion(int x, int y, PARTICLE *particles, float d, int c)
{
    int i,xp,yp,f;
    float tmp;
    PARTICLE *part;

    f=0;
    part = particles;
    for ( i=0; i<NUMPART; i++ )
    {
        // Convert 3d -> 2d
        if ( part->z != 0.0 )
        {
            tmp = d / part->z;
            xp = (x + part->x * tmp);
            yp = (y + part->y * tmp);
        } else {
            xp = x;
            yp = y;
        }

        // Draw particle with clipping
        if ( xp>0 && xp<319 && yp>0 && yp<199)
        {
            buffer[xp+yval[yp]]=128*tmp+c;
            f++;
        }

        // Update particle
        part->x += part->dirx;
        part->y += part->diry;
        part->z += part->dirz;

        // We're on earth!
        part->diry += gravity;

        // Process next particle
        part++;
    }

    return f;
}