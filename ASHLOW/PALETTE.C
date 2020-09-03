#include <math.h>
#include "gfx32.h"
#include "palette.h"

// Phong palette
#define LIGHT           40
#define	REFLECT			150
#define	AMBIENT			0

void SetFirePal1(void)
{
    BYTE i;
    for ( i=0; i<64; i++ )
    {
        destpal[i].r=0;
        destpal[i].g=0;
        destpal[i].b=i/4;

        destpal[64+i].r=i/2;
        destpal[64+i].g=0;
        destpal[64+i].b=16-i/4;

        destpal[128+i].r=32+i/2;
        destpal[128+i].g=i/2;
        destpal[128+i].b=0;

        destpal[192+i].r=63;
        destpal[192+i].g=32+i/2;
        destpal[192+i].b=i;
    }
}

void SetFirePal2(void)
{
    BYTE i;
    for ( i=0; i<64; i++ )
    {
        destpal[i].r=0;
        destpal[i].g=0;
        destpal[i].b=i/2;

        destpal[64+i].r=0;
        destpal[64+i].g=i/3;
        destpal[64+i].b=32+i/4;

        destpal[128+i].r=i/2;
        destpal[128+i].g=21+i/3;
        destpal[128+i].b=48+i/4;

        destpal[192+i].r=32+i/2;
        destpal[192+i].g=42+i/3;
        destpal[192+i].b=63;
    }
}

void SetPal1(void)
{
    BYTE i;
    for ( i=0; i<64; i++ )
    {
        destpal[i].r=i/4;
        destpal[i].g=0;
        destpal[i].b=i/2;

        destpal[64+i].r=16+i/4;
        destpal[64+i].g=0;
        destpal[64+i].b=32+i/2;

        destpal[128+i].r=32+i/4;
        destpal[128+i].g=i/2;
        destpal[128+i].b=63;

        destpal[192+i].r=48+i/4;
        destpal[192+i].g=32+i/2;
        destpal[192+i].b=63;
    }
}

void SetPal2(void)
{
    BYTE i;
    for ( i=0; i<64; i++ )
    {
        destpal[i].r=0;
        destpal[i].g=0;
        destpal[i].b=i/4;

        destpal[64+i].r=i/3;
        destpal[64+i].g=0;
        destpal[64+i].b=16+i/4;

        destpal[128+i].r=21+i/3;
        destpal[128+i].g=i/2;
        destpal[128+i].b=32+i/4;

        destpal[192+i].r=42+i/3;
        destpal[192+i].g=32+i/2;
        destpal[192+i].b=48+i/4;
    }
}

void SetBumpPal1(void)
{
    int loop, temp;
    double intensity;

	// create phong palette
    for ( loop = 0; loop < 256; loop ++ )
    {
        intensity = cos ( (255-loop) / 512.0 * kpi );

        temp = 30 * AMBIENT / 63.0 + 30 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].r = temp;

        temp = 40 * AMBIENT / 63.0 + 40 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].g = temp;

        temp = 17 * AMBIENT / 63.0 + 17 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].b = temp;
    }
}

void SetBumpPal2(void)
{
    int loop, temp;
    double intensity;

    // create phong palette
    for ( loop = 0; loop < 256; loop ++ )
    {
        intensity = cos ( (255-loop) / 512.0 * kpi );

        temp = 12 * AMBIENT / 63.0 +  12 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].r = temp;

        temp = 30 * AMBIENT / 63.0 + 30 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].g = temp;

        temp = 50 * AMBIENT / 63.0 + 50 * intensity + pow ( intensity, REFLECT ) * LIGHT;
		if ( temp > 63 )
			temp = 63;
        destpal[loop].b = temp;
    }
}