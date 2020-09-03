/*-----------------02/05/98 14.31-------------------
 Fractal mania C++ version
--------------------------------------------------*/
//#define AMOUNT_OF_MATH

#include <conio.h>
#include <time.h>
#include "ptc.h"
#include "fract.hpp"

void Bye(void);

int main(int argc,char *argv[])
{
    Fractal JFract;
    clock_t start,end;
    double fps;
    long frames=0;
    long size;
    uchar *buffer;

    // initialize from command line (ie. "nameprog RGB565")
    PTC ptc(320,240,argc,argv);
    if (!ptc.ok())
    {
        // fallback to defualt mode
        if (!ptc.Init(320,240,RGB565))
        {
            // failure
            ptc.Error("could not initialize ptc");
            return 1;
        }
    }

    // Create surface
    Surface surface(ptc,320,240,ARGB8888);
    if (!surface.ok())
    {
        // failure
        ptc.Error("could not create surface");
        return 1;
    }

    // load image
    surface.Load(ptc,"image.tga");
    if (!surface.ok())
    {
        // failure
        ptc.Error("could not load image");
        return 1;
    }


    // convert image
    if (!surface.Convert(ARGB8888))
    {
        // failure
        ptc.Error("could not convert image");
        return 1;
    }

    // create fractal surface, specifically for be used with the fast render
    Surface fsurf(ptc,128,128,ARGB8888);

    // get info
    uint depth  = fsurf.GetPitch();
    uint bpp    = fsurf.GetBytesPerPixel();

    // lock fractal surface
    buffer=(uchar*)fsurf.Lock();
    if (!buffer)
    {
        // failure
        ptc.Error("could not lock surface");
        return 1;
    }

    size=128*128*bpp;
    frames=0;
    start=clock();
    while (!ptc.kbhit())
    {
        frames++;

        // clear fractal surface
        memset(buffer,0,size);

        // select fractal
        JFract.AutoSet();

        // render fractal
        JFract.Render(buffer,depth,bpp);

        // update to "surface" (which is what you see on video)
        fsurf.BitBlt(surface,96,56);
        surface.Update();
    }
    end=clock();

    // unlock fractal surface
    fsurf.Unlock();

    // close ptc
    ptc.Close();

    fps=(frames*1000)/(end-start);
    cout << "Frames generated  : " << frames << endl;
    cout << "Seconds elapsed   : " << (end-start)/1000 << endl;
    cout << "Frames per second : " << fps << endl << endl;
    #ifdef AMOUNT_OF_MATH
        cout << "Iteration for Julia     : " << JFract.jm << endl;
        cout << "Iteration for Julibrot  : " << JFract.jbm << endl;
        cout << "Iteration for Mandelbrot: " << JFract.mm << endl << endl;
    #endif

    Bye();
    return 0;
}

//
// Messaggio finale
//
void Bye(void)
{
    cout << "Fractal Mania! (c) 1998 by Manowar of Rolling Pixels.\n";
    cout << "Visit our page at:\n";
    cout << "   www.geocities.com\\SiliconValley\\Way\\2147  \n";
    cout << "\nIf you want contact us please send an e-mail to:\n";
    cout << "   manowar-rps@usa.net \n";
    cout << "   deda-rps@usa.net \n";
    cout << "\nor, if you are on FidoNet to:\n";
    cout << "   2:331\\304.17 (MANOWAR)\n";
    cout << "\nor contact one of thiz BBS:\n";
    cout << "   LANDOVER    : 39-2-6122203 \n";
    cout << "   USS STATION : 39-2-39003978 \n";
    cout << "and leave me a message.\n\n";
}