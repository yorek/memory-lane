/*-----------------02/05/98 14.27-------------------
 Fractal class by Manowar of Rolling pixels.
 Need the PTC graphics library.

 Version 0.1
  - Base. Only renders Julia set, end only with a
    SLOOOOOW floating point routine.
    Only surface supported is a 32bit surface.

Version 0.2
  - Fast (but less accurate) render routine
    implemented. This is strictly limited
    to an area of 128x128 pixel with *NO*
    possibility of real-time zooming.
    This metod internally uses a GREY8 surface.

 Version 0.3
  - Fixed a silly bug that dind't let the fast
    routine draw correctly the fractal while using
    ARGB8888 or RGB565 or similar instead of
    RGB332 or GREY8.
  - Now support every screen mode.
  - Added the Julibrot routine (only fast render).

 Verision 1.0
  - Class revisited. Modified to simplify callings.
  - Added the Mandelbrot routine.
  - Julibrot routine extended to a more accurate render.

 Version 1.0b
  - When using the accurate routine, it's now possible
    to choose rendering area in the gauss plane

 Version 1.0c
  - Now the precision value can be set to ANY value
    without having colour's trouble

--------------------------------------------------*/

class Fractal
{
    int x,y;
    float x0,y0,x1,y1;
    float xs,ys;
    float r,c;
    int fx0,fy0,fx1,fy1;
    int fxs,fys;
    int fr,fc;
    int i;
    float ang,ang2,ic;
    uint colr,colg,colb;

public:
    #ifdef AMOUNT_OF_MATH
        uint jm,jbm,mm;
    #endif

    Fractal(void);

    // Dynamic update of fractal (to do animations). Only available for the integer render
    AutoSet(void);

    // Manually set the paramters for Fractal and Julibrot
    Set(float v1, float v2);
    Set(int v1, int v2);

    // The first routine uses an accurate and a versatile but slow algorithm,
    // the second uses a fast integer high-optimized algorithm (only 128x128 pixels)
    Render(uchar *buffer, uint width, uint height, float sx, float ex, float sy, float ey, uint prec, uint depth, uint bpp);
    Render(uchar *buffer, uint depth, uint bpp);

    Julibrot_Render(uchar *buffer, uint width, uint height, float sx, float ex, float sy, float ey, uint prec, uint depth, uint bpp);
    Julibrot_Render(uchar *buffer, uint depth, uint bpp);

    Mandel_Render(uchar *buffer, uint width, uint height, float sx, float ex, float sy, float ey, uint prec, uint depth, uint bpp);
    Mandel_Render(uchar *buffer, uint depth, uint bpp);

private:
    inline SetColor(uint i);
};

Fractal::Fractal(void)
{
    x=y=0;
    x0=y0=x1=y1=x1=xs=0;
    r=c=0;
    fx0=fy0=fx1=fy1=fx1=fxs=0;
    fr=fc=0;
    ang=ang2=rand()/10000;
    ic=0.01;
    colr=colg=colb=0;
    #ifdef AMOUNT_OF_MATH
        jm=jbm=mm=0;
    #endif
}

Fractal::AutoSet(void)
{
    fc=55535*cos(ang)+16384*cos(ang2);
    fr=55535*sin(ang)+fc*sin(ang2);

    ang=ang+0.001;
    if ( ang>6.283185 ) ang=ang-6.283185;

    ang2=ang2+0.007;
    if ( ang2>6.283185 ) ang2=ang2-6.283185;
}

Fractal::Set(float v1, float v2)
{
    r=v1;
    c=v2;
}

Fractal::Set(int v1, int v2)
{
    fr=v1;
    fc=v2;
}

Fractal::Render(uchar *buffer, uint width, uint height,float sx, float ex,float sy, float ey,uint prec, uint depth, uint bpp)
{
    float dx=width/(ex-sx);
    float dy=height/(ey-sy);

    // Render
    for ( x=0; x<width; x++ )
    {
        xs=sx+x/dx;
        for ( y=0; y<height; y++ )
        {
            x0=xs;
            y0=sy+y/dy;
            for ( i=0; i<prec; i++)
            {
                x1=x0*x0-y0*y0+r;
                y1=2*x0*y0+c;

                #ifdef AMOUNT_OF_MATH
                    jm++;
                #endif

                if ((x1*x1 + y1*y1)>4) break;
                x0=x1;
                y0=y1;
            }
            uint *pixel=(uint*)(buffer+(x*bpp)+(y*depth));
            SetColor(i);
            if ( i==prec ) colr=colg=colb=255;
            *pixel=RGB32(colr,colg,colb);
        }
    }
}

Fractal::Render(uchar *buffer, uint depth, uint bpp)
{
    uint offy,offx,offx2,offy2,sust,sust2;
    int dfx,dfy;
    uint *pixel;

    offx=0;
    offx2=128*bpp;
    sust=128*depth;

    // Render
    fxs=-512;
    for ( x=0; x<128; x++ )
    {
        fys=-512;
        offy=0;
        offy2=sust;
        sust2=fxs*fxs;
        for ( y=0; y<=64; y++ )
        {
            fx0=fxs;
            fy0=fys;

            dfy=fy0*fy0;
            dfx=sust2;
            for ( i=0; i<=248; i+=5)
            {
                fx1=dfx-dfy+fr;
                fy1=fx0*fy0;
                fy1+=fy1;
                fy1+=fc;

                fx1>>=8;
                fy1>>=8;

                dfx=fx1*fx1;
                dfy=fy1*fy1;

                if ((dfx + dfy)>262144) break;
                fx0=fx1;
                fy0=fy1;
            }
            if (i>0) {
                pixel=(uint*)(buffer+offx+offy);
                *pixel=RGB32(i,i,i);

                pixel=(uint*)(buffer+offx2+offy2);
                *pixel=RGB32(i,i,i);
                if ( i>248 )
                {
                    pixel=(uint*)(buffer+offx+offy);
                    *pixel=RGB32(i,i,i);

                    pixel=(uint*)(buffer+offx2+offy2);
                    *pixel=RGB32(i,i,i);
                }
            }

            fys+=8;
            offy+=depth;
            offy2-=depth;
        }
        fxs+=8;
        offx+=bpp;
        offx2-=bpp;
    }
}

Fractal::Julibrot_Render(uchar *buffer, uint width, uint height,float sx, float ex,float sy, float ey,uint prec, uint depth, uint bpp)
{
    float dx=width/(ex-sx);
    float dy=height/(ey-sy);

    // Render
    for ( x=0; x<width; x++ )
    {
        xs=sx+x/dx;
        for ( y=0; y<height; y++ )
        {
            x0=xs;
            y0=sy+y/dy;
            for ( i=0; i<prec; i++)
            {
                x1=x0*x0-y0*y0+r+xs;
                y1=2*x0*y0+c+ys;

                #ifdef AMOUNT_OF_MATH
                    jbm++;
                #endif

                if ((x1*x1 + y1*y1)>4) break;
                x0=x1;
                y0=y1;
            }
            uint *pixel=(uint*)(buffer+(x*bpp)+(y*depth));
            SetColor(i);
            if ( i==prec ) colr=colg=colb=255;
            *pixel=RGB32(colr,colg,colb);
        }
    }
}

Fractal::Julibrot_Render(uchar *buffer, uint depth, uint bpp)
{
    uint offy,offx,offx2;
    int dfx,dfy;
    uint *pixel;

    offx=0;

    // Render
    fxs=-512;
    for ( x=0; x<128; x++ )
    {
        fys=-512;
        offy=0;
        for ( y=0; y<128; y++ )
        {
            fx0=fxs;
            fy0=fys;

            dfx=fx0*fx0;
            dfy=fy0*fy0;
            for ( i=0; i<=248; i+=5)
            {
                fx1=dfx-dfy+fr+(fxs<<7);
                fy1=((fx0*fy0)<<1)+fc+(fys<<7);

                fx1>>=8;
                fy1>>=8;

                dfx=fx1*fx1;
                dfy=fy1*fy1;

                if ((dfx + dfy)>262144) break;
                fx0=fx1;
                fy0=fy1;
            }
            if (i>0) {
                pixel=(uint*)(buffer+offx+offy);
                *pixel=RGB32(i,i,i);

                if ( i>248 )
                {
                    pixel=(uint*)(buffer+offx+offy);
                    *pixel=RGB32(i,i,i);
                }
            }

            fys+=8;
            offy+=depth;
        }
        fxs+=8;
        offx+=bpp;
    }
}

Fractal::Mandel_Render(uchar *buffer, uint width, uint height,float sx, float ex,float sy, float ey,uint prec, uint depth, uint bpp)
{
    float dx=width/(ex-sx);
    float dy=height/(ey-sy);

    // Render
    for ( x=0; x<width; x++ )
    {
        xs=sx+x/dx;
        for ( y=0; y<height; y++ )
        {
            x0=xs;
            y0=ys=sy+y/dy;
            for ( i=0; i<prec; i++)
            {
                x1=x0*x0-y0*y0+xs;
                y1=2*x0*y0+ys;

                #ifdef AMOUNT_OF_MATH
                    mm++;
                #endif

                if ((x1*x1 + y1*y1)>prec) break;
                x0=x1;
                y0=y1;
            }
            uint *pixel=(uint*)(buffer+(x*bpp)+(y*depth));
            SetColor(i);
            if ( i==prec ) colr=colg=colb=255;
            *pixel=RGB32(colr,colg,colb);
        }
    }
}

Fractal::Mandel_Render(uchar *buffer, uint depth, uint bpp)
{
    uint offy,offx,offx2;
    int dfx,dfy;
    uint *pixel;

    offx=0;

    // Render
    fxs=-512;
    for ( x=0; x<128; x++ )
    {
        fys=-512;
        offy=0;
        for ( y=0; y<128; y++ )
        {
            fx0=fxs;
            fy0=fys;

            dfx=fx0*fx0;
            dfy=fy0*fy0;
            for ( i=0; i<=248; i+=5)
            {
                fx1=dfx-dfy+(fxs<<8);
                fy1=((fx0*fy0)<<1)+(fys<<8);

                fx1>>=8;
                fy1>>=8;

                dfx=fx1*fx1;
                dfy=fy1*fy1;

                if ((dfx + dfy)>262144) break;
                fx0=fx1;
                fy0=fy1;
            }
            if (i>0) {
                pixel=(uint*)(buffer+offx+offy);
                *pixel=RGB32(i,i,i);

                if ( i>248 )
                {
                    pixel=(uint*)(buffer+offx+offy);
                    *pixel=RGB32(i,i,i);
                }
            }

            fys+=8;
            offy+=depth;
        }
        fxs+=8;
        offx+=bpp;
    }
}

inline Fractal::SetColor(uint i)
{
    colr=abs(sin(i*0.017453)*255);
    colg=abs(sin(i*0.034906)*255);
    colb=abs(sin(i*0.069813)*255);
}