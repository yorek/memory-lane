/*-----------------11/06/98 19.27-------------------
    Fractal Class Library
--------------------------------------------------*/
#include "fract.h"
#include <math.h>

#define FCL "Fractal Class Library v 1.0 (c) Manowar / Rolling Pixels 1998"

/*-----------------18/06/98 15.50-------------------
 Base class functions
--------------------------------------------------*/
Fractal::Fractal(void)
{
    dx=dy=0;
    sx=ex=sy=ey=0;
    x=y=0;
    x0=y0=x1=y1=0;
    xp=yp=0;
    r=c=0;
    bs=4;
    iter=totit=0;
    start=end=0;
    colortable = new uint [65535];
    blcy=blcx=0;
    color=0;
    colormode=0;
    clm=0;
}

Fractal::~Fractal(void)
{
    delete colortable;
}

Fractal::LibInfo(void)
{
    cout << FCL << endl;
}

Fractal::CreateColorTable(void)
{
    uint c=0,z=0, z2=0;
    for ( z=0; z<32; z++, z2+=6 )
    {
        for ( c=0; c<256; c++ )
        {
            colortable[2048*z + c]       =   RGB32(c,        z2,     z2);
            colortable[2048*z + 256+c]   =   RGB32(255,      c,      z2);
            colortable[2048*z + 512+c]   =   RGB32(255-c,    255,    z2);
            colortable[2048*z + 768+c]   =   RGB32(z2,       255-c,  c);
            colortable[2048*z + 1024+c]  =   RGB32(c,        z2,     255);
            colortable[2048*z + 1280+c]  =   RGB32(255-c,    c,      255);
            colortable[2048*z + 1280+c]  =   RGB32(255-c,    c,      255);
            colortable[2048*z + 1536+c]  =   RGB32(z2,       255-c,  255);
            colortable[2048*z + 1792+c]  =   RGB32(z2,       z2,     255-c);
        }
    }
}

char Fractal::CheckAllocations(void)
{
    if ( !colortable ) return 0;
    return 1;
}

Fractal::ResetCounter(void)
{
    totit=0;
    start=end=0;
}

Fractal::ShowCounter(void)
{
    cout << "Total iteration : " << totit << endl;
    cout << "Time Elapsed    : " << (float)(end-start)/1000.0 << " secs" << endl;
}

Fractal::StartCounter(void)
{
    start=clock();
}

Fractal::StopCounter(void)
{
    end=clock();
}

Fractal::SetStartBlockSize(uint size)
{
    bs=size;
}

Fractal::SetColorMode(uint cm)
{
    colormode=cm;
}

Fractal::StoreCoord(uint width, uint height, double x1, double x2, double y1, double y2)
{
    sx=x1;
    ex=x2;
    sy=y1;
    ey=y2;

    if ( sx>ex )
    {
        dx=sx;
        sx=ex;
        ex=dx;
    }
    if ( sy<ey )
    {
        dy=sy;
        sy=ey;
        ey=dy;
    }

    dx=(ex-sx)/width;
    dy=(ey-sy)/height;
}

Fractal::AdjustColorLimits(void)
{
    switch ( colormode )
    {
        case BY_ITER:
                {
                    clm=0;
                    break;
                }
        case BY_MAX:
                {
                    blcx=fabs(sx);
                    if ( fabs(ex)>blcx ) blcx=fabs(ex);
                    if ( fabs(sy)>blcx ) blcx=fabs(sy);
                    if ( fabs(ey)>blcx ) blcx=fabs(ey);
                    blcy=blcx;
                    clm=1;
                    break;
                }
        case BY_RELATIVE :
                {
                    blcx=fabs(sx);
                    if ( fabs(ex)>blcx ) blcx=fabs(ex);
                    blcy=fabs(sy);
                    if ( fabs(ey)>blcy ) blcy=fabs(ey);
                    clm=1;
                    break;
                }
    }
}

inline Fractal::SetPerturbation(double v1, double v2)
{
    r=v1;
    c=v2;
}

inline Fractal::RenderPoint(void)
{
    x0=xp;
    y0=yp;
    for ( iter=0; iter<prec; iter++)
    {
        x1=(x0*x0)-(y0*y0)+r;
        y1=(2*x0*y0)+c;

        if ((x1*x1 + y1*y1)>4) break;

        x0=x1;
        y0=y1;
    }
    totit+=iter;
}

inline Fractal::DrawPoint(void)
{
    if ( iter!=prec )
    {
        color=clm*iter*((blcx-xp*xp)+(blcy-yp*yp))+iter;
        *pixel=colortable[color];
    } else {
        *pixel=RGB32(255,255,255);
    }
}

/*-----------------18/06/98 15.50-------------------
 Mandelbrot Class specific functions
--------------------------------------------------*/

// 1 Pass render.
// Manually update to screen
Mandelbrot::Render(Surface &surf, uchar *buffer, uint width, uint height, double x1, double x2, double y1, double y2, uint pc)
{
    uint p=0;
    uint wp,hp,co;

    uint bpp=surf.GetBytesPerPixel();
    uint depth=surf.GetPitch();
    uint *ap,*bp,*cp,*px;

    // Memorize and check coordinates
    StoreCoord(width,height,x1,x2,y1,y2);

    // Memorize requested recision
    prec=pc;

    // Calculate color limits
    AdjustColorLimits();

    // Render
    ResetCounter();
    StartCounter();
    for ( p=bs; p>0; p>>=1 )
    {
        yp=sy;
        for ( y=0; y<height; y+=p )
        {
            xp=sx;
            for ( x=0; x<width; x+=p )
            {
                pixel=(uint*)(buffer+(x*bpp)+(y*depth));

                if ( *pixel==0 )
                {
                    // Set the right values for Mandelbrot Set
                    SetPerturbation(xp,yp);

                    // Get the number of iterations for current pixel
                    RenderPoint();

                    // Draw it accordingly to colormode and its iter number
                    DrawPoint();

                    // If can predict the value of an entire area fill it with
                    // prediction value
                    if ( x>=p && y>=p && p>1)
                    {
                        co =*pixel;
                        ap =(uint*)(buffer+((x-p)*bpp)+((y-p)*depth));
                        bp =(uint*)(buffer+(x*bpp)+((y-p)*depth));
                        cp =(uint*)(buffer+((x-p)*bpp)+(y*depth));

                        if ( *ap==co && *bp==co && *cp==co )
                        {
                            for ( hp=y-p; hp<=y; hp++ )
                                for ( wp=x-p; wp<=x; wp++ )
                                {
                                    px=(uint*)(buffer+(wp*bpp)+(hp*depth));
                                    *px=co;
                                }

                        }

                    }
                }
                xp=xp+dx*p;
            }
            yp=yp+dy*p;
            surf.Update(RECTANGLE(0,y-p,width-1,y));
        }
    }
    StopCounter();
}

// 1 Pass render.
// Auto update to screen
char Mandelbrot::Render(Surface &surf, uint width, uint height, double x1, double x2, double y1, double y2, uint pc)
{
    uint p=0;
    uint wp,hp,co;

    uint bpp=surf.GetBytesPerPixel();
    uint depth=surf.GetPitch();
    uint *ap,*bp,*cp,*px;

    // lock surface
    uchar *buffer=(uchar*)surf.Lock();
    if (!buffer)
    {
        // failure
        return 0;
    }

    // Memorize and check coordinates
    StoreCoord(width,height,x1,x2,y1,y2);

    // Memorize requested recision
    prec=pc;

    // Calculate color limits
    AdjustColorLimits();

    // Render
    ResetCounter();
    StartCounter();
    for ( p=bs; p>0; p>>=1 )
    {
        yp=sy;
        for ( y=0; y<height; y+=p )
        {
            xp=sx;
            for ( x=0; x<width; x+=p )
            {
                pixel=(uint*)(buffer+(x*bpp)+(y*depth));

                if ( *pixel==0 )
                {
                    // Set the right values for Mandelbrot Set
                    SetPerturbation(xp,yp);

                    // Get the number of iterations for current pixel
                    RenderPoint();

                    // Draw it accordingly to colormode and the iter number
                    DrawPoint();
                }
                // If can predict the value of an entire area fill it with
                // prediction value
                if ( x>=p && y>=p && p>1)
                {
                    co =*pixel;
                    ap =(uint*)(buffer+((x-p)*bpp)+((y-p)*depth));
                    bp =(uint*)(buffer+(x*bpp)+((y-p)*depth));
                    cp =(uint*)(buffer+((x-p)*bpp)+(y*depth));

                    if ( *ap==co && *bp==co && *cp==co )
                    {
                        for ( hp=y-p; hp<=y; hp++ )
                            for ( wp=x-p; wp<=x; wp++ )
                            {
                                px=(uint*)(buffer+(wp*bpp)+(hp*depth));
                                *px=co;
                            }

                    }

                }
                xp=xp+dx*p;
            }
            yp=yp+dy*p;
            surf.Update(RECTANGLE(0,y-p,width-1,y));
        }
    }
    StopCounter();

    // unlock surface
    surf.Unlock();

    return 1;
}

// 2 Pass Render.
// Manually update to screen
Mandelbrot::H_Render(Surface &surf, uchar *buffer, uint width, uint height, double x1, double x2, double y1, double y2, uint pc)
{
    uint p=0;
    uint wp,hp,co;

    uint bpp=surf.GetBytesPerPixel();
    uint depth=surf.GetPitch();
    uint *ap,*bp,*cp,*px;

    // Memorize and check coordinates
    StoreCoord(width,height,x1,x2,y1,y2);

    // Memorize requested precision
    prec=pc;

    // Calculate color limits
    AdjustColorLimits();

    // Render
    ResetCounter();
    StartCounter();
    for ( p=bs; p>0; p>>=1 )
    {
        yp=sy;
        for ( y=0; y<height; y+=p )
        {
            xp=sx;
            for ( x=0; x<width; x+=p )
            {
                pixel=(uint*)(buffer+(x*bpp)+(y*depth));

                if ( *pixel==0 )
                {
                    // Set the right values for Mandelbrot Set
                    SetPerturbation(xp,yp);

                    // Get the number of iterations for current pixel
                    RenderPoint();

                    // Store iteration for this pixel
                    *pixel=iter;

                    // If can predict the value of an entire area fill it with
                    // prediction value
                    if ( x>=p && y>=p && p>1)
                    {
                        co =*pixel;
                        ap =(uint*)(buffer+((x-p)*bpp)+((y-p)*depth));
                        bp =(uint*)(buffer+(x*bpp)+((y-p)*depth));
                        cp =(uint*)(buffer+((x-p)*bpp)+(y*depth));

                        if ( *ap==co && *bp==co && *cp==co )
                        {
                            for ( hp=y-p; hp<=y; hp++ )
                                for ( wp=x-p; wp<=x; wp++ )
                                {
                                    px=(uint*)(buffer+(wp*bpp)+(hp*depth));
                                    *px=co;
                                }

                        }

                    }
                }
                xp=xp+dx*p;
            }
            yp=yp+dy*p;
            surf.Update(RECTANGLE(0,y-p,width-1,y));
        }
    }
    StopCounter();

    // Post colorize
    yp=sy;
    for ( y=0; y<height; y++ )
    {
        xp=sx;
        for ( x=0; x<width; x++ )
        {
            pixel=(uint*)(buffer+(x*bpp)+(y*depth));
            iter=*pixel;
            DrawPoint();
            xp=xp+dx;
        }
        surf.Update(RECTANGLE(0,y-1,width-1,y));
        yp=yp+dy;
    }
}

// 2 Pass Render.
// Auto update to screen
char Mandelbrot::H_Render(Surface &surf, uint width, uint height, double x1, double x2, double y1, double y2, uint pc)
{
    uint p=0;
    uint wp,hp,co;

    uint bpp=surf.GetBytesPerPixel();
    uint depth=surf.GetPitch();
    uint *ap,*bp,*cp,*px;

    // lock surface
    uchar *buffer=(uchar*)surf.Lock();
    if (!buffer)
    {
        // failure
        return 0;
    }

    // Memorize and check coordinates
    StoreCoord(width,height,x1,x2,y1,y2);

    // Memorize requested precision
    prec=pc;

    // Calculate color limits
    AdjustColorLimits();

    // Render
    ResetCounter();
    StartCounter();
    for ( p=bs; p>0; p>>=1 )
    {
        yp=sy;
        for ( y=0; y<height; y+=p )
        {
            xp=sx;
            for ( x=0; x<width; x+=p )
            {
                pixel=(uint*)(buffer+(x*bpp)+(y*depth));

                if ( *pixel==0 )
                {
                    // Set the right values for Mandelbrot Set
                    SetPerturbation(xp,yp);

                    // Get the number of iterations for current pixel
                    RenderPoint();

                    // Store iteration for this pixel
                    *pixel=iter;
                }
                // If can predict the value of an area fill it with
                // prediction value
                if ( x>=p && y>=p && p>1)
                {
                    co =*pixel;
                    ap =(uint*)(buffer+((x-p)*bpp)+((y-p)*depth));
                    bp =(uint*)(buffer+(x*bpp)+((y-p)*depth));
                    cp =(uint*)(buffer+((x-p)*bpp)+(y*depth));

                    if ( *ap==co && *bp==co && *cp==co )
                    {
                        for ( hp=y-p; hp<=y; hp++ )
                            for ( wp=x-p; wp<=x; wp++ )
                            {
                                px=(uint*)(buffer+(wp*bpp)+(hp*depth));
                                *px=co;
                            }

                    }

                }
                xp=xp+dx*p;
            }
            yp=yp+dy*p;
            surf.Update(RECTANGLE(0,y-p,width-1,y));
        }
    }
    StopCounter();

    // Post colorize
    yp=sy;
    for ( y=0; y<height; y++ )
    {
        xp=sx;
        for ( x=0; x<width; x++ )
        {
            pixel=(uint*)(buffer+(x*bpp)+(y*depth));
            iter=*pixel;
            DrawPoint();
            xp=xp+dx;
        }
        surf.Update(RECTANGLE(0,y-1,width-1,y));
        yp=yp+dy;
    }

    // unlock surface
    surf.Unlock();

    return 1;
}