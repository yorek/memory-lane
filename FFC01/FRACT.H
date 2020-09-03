/*-----------------11/06/98 19.27-------------------
    Fast Fractal Class
--------------------------------------------------*/
#include "ptc.h"

#define BY_ITER     0
#define BY_MAX      1
#define BY_RELATIVE 2

class Fractal
{
protected:
    // Screen coordinates
    int x,y;

    // Fractal temp vaule
    double x0,y0,x1,y1;

    // Limit of the gaussian plane
    double sx,ex,sy,ey;

    // Step in gaussian plane (discretization value)
    double dx,dy;

    // Coordinates in the gaussian plane
    double xp,yp;

    // Perturbation value
    double r,c;

    // Buondary Limits for coloring fractals.
    double blcx,blcy;

    // Itertions values
    uint iter,totit,prec;

    // Block Size
    uint bs;

    // Curret pixel color, and colormode
    uint color,colormode;

    // Colormode multipler
    double clm;

    // Memory Location surface
    uint *pixel;

    // Look-up color table
    uint *colortable;

    // Timings var.
    clock_t start,end;

public:
    Fractal(void);
    ~Fractal(void);

    // Print out library info
    LibInfo(void);

    // Check if memory was allocated correctly
    char CheckAllocations(void);

    // Create the color look-up
    CreateColorTable(void);

    // Show time used and iteration counter
    ShowCounter(void);

    // Set Block Size
    SetStartBlockSize(uint size);

    // Set color mode
    SetColorMode(uint cm);

protected:
    // Start timer
    StartCounter(void);

    // Stop timer
    StopCounter(void);

    // Reset timer and iteration counter
    ResetCounter(void);

    // Store the coordinated selected for the gaussian plane and calculates the discretization value
    StoreCoord(uint width, uint height, double x1, double x2, double y1, double y2);

    // Adjust color variables for the selected colormode
    AdjustColorLimits(void);

    // Draw the point at memory location addressed by "pixel" on screen
    inline DrawPoint(void);

    // Render the point
    inline RenderPoint(void);

    // Set value of r,c
    inline SetPerturbation(double v1, double v2);
};

class Mandelbrot : public Fractal
{
public:
    // 1 pass Render
    // Manual update to video display (Must call Look and Unlook manually)
    Render( Surface &surf, uchar *buffer,
            uint width, uint height,
            double x1, double x2, double y1, double y2,
            uint pc);
    // Auto update to video display
    char Render(Surface &surf,
            uint width, uint height,
            double x1, double x2, double y1, double y2,
            uint pc);

    // 2 pass render
    H_Render( Surface &surf, uchar *buffer,
            uint width, uint height,
            double x1, double x2, double y1, double y2,
            uint pc);
    char H_Render(Surface &surf,
            uint width, uint height,
            double x1, double x2, double y1, double y2,
            uint pc);
};