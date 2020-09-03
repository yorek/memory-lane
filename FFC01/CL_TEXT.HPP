/* -----------------15/08/98 21.18-------------------
Text Class library.
Working surface should be ARGB8888
--------------------------------------------------*/
#include "ptc.h"
#include "sfont.h"
#include "bfont.h"

#define SMALL_FONT          0
#define BIG_FONT            1


class GRAPH_TEXT
{
private:
    char r,g,b;             // Color components
    char mode;              // Text printing mode
    char font;              // font type
    int pitch,bpp;          // Surface info
    int x,y;                // Text coordinates
    char *buffer;           // Pointer of screen

public:
    GRAPH_TEXT();
    GRAPH_TEXT(int vpitch, int vbpp, char *vbuf);
    ~GRAPH_TEXT();

    char Print(char *msg);  // Return 1 if not initialized!
    void Init(int vpitch, int vbpp, char *vbuf);
    void Set_Font(char vfont);
    void Set_Color(char vr, char vg, char vb);
    void Locate(int vx, int vy);

private:
    void Small_Print(char *msg);
    void Big_Print(char *msg);
};

//
// Constructors.
//
GRAPH_TEXT::GRAPH_TEXT()
{
    // Set white for default color
    r=g=b=255;

    // Set small font
    font=SMALL_FONT;

    // Now initialize the other vars to no value
    x=y=0;
    buffer=NULL;
    pitch=bpp=0;
}

GRAPH_TEXT::GRAPH_TEXT(int vpitch, int vbpp, char *vbuf)
{
    // Set white for default color
    r=g=b=255;

    // Set small font
    font=SMALL_FONT;

    // Now initialize the other vars to no value
    x=y=0;

    // Initalize with passed values
    pitch=vpitch;
    bpp=vbpp;
    buffer=vbuf;

}

//
// Desctructor not needed.
//
GRAPH_TEXT::~GRAPH_TEXT()
{
}

//
// Get surface infos
//
void GRAPH_TEXT::Init(int vpitch, int vbpp, char *vbuf)
{
    pitch=vpitch;
    bpp=vbpp;
    buffer=vbuf;
}

//
// Set font
//
void GRAPH_TEXT::Set_Font(char vfont)
{
    font=vfont;
}

//
// Set color
//
void GRAPH_TEXT::Set_Color(char vr, char vg, char vb)
{
    r=vr;
    g=vg;
    b=vb;
}

//
// Set text position
//
void GRAPH_TEXT::Locate(int vx, int vy)
{
    x=vx;
    y=vy;
}

//
// Print
//
char GRAPH_TEXT::Print(char *msg)
{
    if ( buffer==NULL ) return 1;

    switch ( font )
    {
        case SMALL_FONT:    Small_Print(msg); break;
        case BIG_FONT:      Big_Print(msg); break;
    }

    return 0;
}

//
// Print with small font
//
void GRAPH_TEXT::Small_Print(char *msg)
{
    uint xc,yc;
    char v,l;
    uint *pixel;

    for (l=0; l<strlen(msg); l++  )
    {
        v=msg[l]-32;
        for ( yc=0; yc<7; yc++)
            for ( xc=0; xc<4; xc++ )
                if ( sfont[ v*5 + xc + yc*490]==1 )
                {
                    pixel=(uint*)(buffer + (y+yc)*pitch + (x+xc)*bpp );
                    *pixel=RGB32(r,g,b);
                }
        x+=5;
    }
}

//
// Print with big font
//
void GRAPH_TEXT::Big_Print(char *msg)
{
    uint xc,yc;
    char v,l;
    uint *pixel;

    for (l=0; l<strlen(msg); l++  )
    {
        v=msg[l]-32;
        for ( yc=0; yc<19; yc++)
            for ( xc=0; xc<12; xc++ )
                if ( bfont[v*12 + xc + yc*1280 + 2]==1 )
                {
                    pixel=(uint*)(buffer + (y+yc)*pitch + (x+xc)*bpp );
                    *pixel=RGB32(r,g,b);
                }
        x+=14;
    }
}







