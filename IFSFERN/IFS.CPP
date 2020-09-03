#include "ptc.h"

#define NX 640
#define NY 480

#define min(a,b)  (((a) < (b)) ? (a) : (b))

double a[4] = {0.0,0.2,-0.15,0.75};
double b[4] = {0.0,-0.26,0.28,0.04};
double c[4] = {0.0,0.23,0.26,-0.04};
double d[4] = {0.16,0.22,0.24,0.85};
double e[4] = {0.0,0.0,0.0,0.0};
double f[4] = {0.0,1.6,0.44,1.6};

int main(int argc,char *argv[])
{
    int i,j,k;
    int n = 1000;
    int ix,iy;
    double r,x,y,xlast=0,ylast=0;
    double xmin=1e32,xmax=-1e32,ymin=1e32,ymax=-1e32,scale,xmid,ymid;

    // initialize ptc from command line (eg: "rand32 640 480 ARGB8888")
    PTC ptc(640,480,RGB565);
    if (!ptc.ok())
    {
        // fallback to virtual 32bit
        if (!ptc.Init(320,200))
        {
            // failure
            ptc.Error("could not initialize");
            return 1;
        }
    }

    // get display resolution
    int xres=ptc.GetXResolution();
    int yres=ptc.GetYResolution();

    // create fullscreen surface
    Surface surface(ptc,xres,yres,ARGB8888);
    if (!surface.ok())
    {
        // failure
        ptc.Error("could not create surface");
        return 1;
    }

    // lock surface
    char *buffer=(char*)surface.Lock();
    if (!buffer)
    {
        // failure
        ptc.Error("could not lock surface");
        return 1;
    }

    int pitch=surface.GetPitch();
    int bpp=surface.GetBytesPerPixel();

    // main loop
    while (!ptc.kbhit())
    {
       for (j=0;j<2;j++)
       {
            for (i=0;i<n;i++)
            {
                r = rand() % 100;
                if (r < 10)
                    k = 0;
                else if (r < 18)
                    k = 1;
                else if (r < 26)
                    k = 2;
                else
                    k = 3;

                x = a[k] * xlast + b[k] * ylast + e[k];
                y = c[k] * xlast + d[k] * ylast + f[k];
                xlast = x;
                ylast = y;
                if (x < xmin) xmin = x;
                if (y < ymin) ymin = y;
                if (x > xmax) xmax = x;
                if (y > ymax) ymax = y;

                if (j == 1)
                {
                    scale = min( NX / (xmax - xmin), NY / (ymax - ymin) );
                    xmid = (xmin + xmax) / 2;
                    ymid = (ymin + ymax) / 2;
                    ix = NX / 2 + (x - xmid) * scale;
                    iy = NY - (NY / 2 + (y - ymid) * scale);

                    if ( ix>=0 && ix<640 && iy>=0 && iy<480 )
                    {
                        uint *pixel=(uint*)(buffer+iy*pitch+ix*bpp);
                        if ( *pixel<RGB32(0,255,0) )
                            *pixel += RGB32(0,1,0);
                    }

                }
            }
        }

        // update to display
        surface.Update();
    }

    // unlock surface
    surface.Unlock();

    return 0;
}