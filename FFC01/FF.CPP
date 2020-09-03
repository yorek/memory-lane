/*-----------------02/05/98 14.31-------------------
 Fast Fractal Explorer
--------------------------------------------------*/

#include <time.h>
#include "ptc.h"
#include "fract.h"

void Bye(void);

int main(int argc,char *argv[])
{
    Mandelbrot Mandel;
    double xs,xe,ys,ye;
    uint cms=0;
    uint bss=4;
    uint wdt=1024;
    uint hgt=768;
    uint itr=1024;
    uint md=2;
    uint tpe=0;
    double jc=0,jr=0;
    char exitnow=0;
    int i;

    xs=-2.5;
    xe=1.5;
    ys=1.5;
    ye=-1.5;

    cout << endl;
    for ( i=1; i<argc; i++ )
    {
        if ( argv[i][0]=='c' || argv[i][0]=='C' )
        {
            cms=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='b' || argv[i][0]=='B' )
        {
            bss=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='w' || argv[i][0]=='W' )
        {
            wdt=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='h' || argv[i][0]=='H' )
        {
            hgt=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='i' || argv[i][0]=='I' )
        {
            itr=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='m' || argv[i][0]=='M' )
        {
            md=atoi(&argv[i][1]);
        }

        if ( argv[i][0]=='t' || argv[i][0]=='T' )
        {
            if (argv[i][1]=='M' || argv[i][1]=='m') tpe=0;
            if (argv[i][1]=='J' || argv[i][1]=='j')
            {
                tpe=1;
                jr=atof(argv[i+1]);
                jc=atof(argv[i+2]);
            }
        }

        if ( argv[i][0]=='?' )
        {
            exitnow=1;
        }

        if ( argv[i][0]=='l' || argv[i][0]=='L' )
        {
            xs=atof(argv[i+1]);
            xe=atof(argv[i+2]);
            ys=atof(argv[i+3]);
            ye=atof(argv[i+4]);
        }
    }

    cout << "Fastrue Fractal v0.1 (c) Manowar / Rolling Pixels 1998" << endl;
    Mandel.LibInfo();
    cout << endl;
    cout << "[t@] Type           : ";
    if ( tpe==0 ) cout << "MANDELBROT" << endl;
    if ( tpe==1 )
    {
        cout << "JULIA" << endl;
        cout << "     Perturbation value : " << jr << "," << jc << "i" << endl;
    }
    cout << "[c#] Color Mode     : " << cms << endl;
    cout << "[b#] Block Size     : " << bss << endl;
    cout << "[w#] Screen Width   : " << wdt << endl;
    cout << "[h#] Screen Height  : " << hgt << endl;
    cout << "[i#] Iteration #    : " << itr << endl;
    cout << "[m#] Mode           : ";
    if ( md==1 )
    {
        cout << "1 PASS" << endl;
    } else if (md==2) {
        cout << "2 PASS" << endl;
    }
    cout << endl;
    cout << "[l # # # #] Block Limits:" << endl;
    cout << "X: " << xs << " ... " << xe << endl;
    cout << "Y: " << ys << " ... " << ye << endl;
    if ( exitnow==1 ) return 0;
    cout << endl << "Press any key to start." << endl;
    getch();

    // Initialize from command line (ie. "nameprog RGB565")
    PTC ptc(wdt,hgt,ARGB8888);
    if (!ptc.ok())
    {
        // fallback to default mode
        if (!ptc.Init(wdt,hgt,VIRTUAL32))
        {
            // failure
            ptc.Error("could not initialize ptc");
            return 1;
        }
    }

    // Create surface
    Surface surface(ptc,wdt,hgt,ARGB8888);
    if (!surface.ok())
    {
        // failure
        ptc.Error("could not create surface");
        return 1;
    }

    // Check memory for colortable
    if (!Mandel.CheckAllocations())
    {
        // failure
        ptc.Close();
        cout << "Fractal: could not lock memory" << endl;
        return 1;
    }

    // Create Colortable
    Mandel.CreateColorTable();

    // Select guessing level. Higher value means lower precision
    Mandel.SetStartBlockSize(bss);

    // Select color mode
    Mandel.SetColorMode(cms);

    // Render
    if ( md==1 )
    {
        if (!Mandel.Render(surface, wdt, hgt, xs, xe, ys, ye, itr))
        {
            // failure
            ptc.Error("could not create surface");
            return 1;
        }
    } else if (md==2) {
        if (!Mandel.H_Render(surface, wdt, hgt, xs, xe, ys, ye, itr))
        {
            // failure
            ptc.Error("could not create surface");
            return 1;
        }
    }
    // close ptc
    ptc.getch();
    ptc.Close();

    // Show some info
    Mandel.ShowCounter();
    // Show end message
    Bye();
    return 0;
}

//
// Bye bye message
//
void Bye(void)
{
    cout << endl;
    cout << "Fastrue Fractal (c) 1998 by Manowar of Rolling Pixels.\n";
    cout << "Visit our page at:\n";
    cout << "   www.geocities.com\\SiliconValley\\Way\\2147  \n";
    cout << "\nIf you want contact us please send an e-mail to:\n";
    cout << "   manowar-rps@usa.net \n";
    cout << "   deda-rps@usa.net \n";
    cout << "\nor, if you are on FidoNet send a matrix to:\n";
    cout << "   2:331\\303.17 (MANOWAR)\n";
    cout << "\nor contact one of thiz BBS:\n";
    cout << "   LANDOVER    : 39-2-6122203 \n";
    cout << "   USS STATION : 39-2-39003978 \n";
    cout << "and leave me a message.\n\n";
}