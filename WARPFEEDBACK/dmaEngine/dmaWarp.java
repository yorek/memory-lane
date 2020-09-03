package dmaEngine;

/*
    Source and Dest image must be 256x256 px
*/

public class dmaWarp
{
    /* Grid */
    private int GridX[][];
    private int GridY[][];
    private int fuzzy, hfuzzy;
    private int gridBoxes, gridBoxesSize, gridBoxesHalfSize, gridPoints;

    public dmaWarp()
    {
        gridBoxes = 16;
        initGrid();
    }

    public dmaWarp(int granularity)
    {
        gridBoxes = granularity;
        initGrid();
    }

    private void initGrid()
    {
        int x,y,xs,ys;

        gridPoints          = gridBoxes + 1;
        gridBoxesSize       = 256 / gridBoxes;
        gridBoxesHalfSize   = gridBoxesSize / 2;

        fuzzy=3;
        hfuzzy=1;

        GridX=new int[gridPoints][gridPoints];
        GridY=new int[gridPoints][gridPoints];

        for (x=0; x<=gridBoxes; x++)
        {
            for (y=0; y<=gridBoxes; y++)
            {
                xs=x*gridBoxesSize;
                if (xs>255) xs=255;

                ys=y*gridBoxesSize;
                if (ys>255) ys=255;

                GridX[x][y] = xs;
                GridY[x][y] = ys;
            }
        }
    }

    public int MeltSize(int melt)
    {
        melt|=1;
        if ((melt<=0)||(melt>gridBoxesSize)) {
            fuzzy=(gridBoxesHalfSize/2)+1;
            fuzzy|=1;
        }
        hfuzzy = ((fuzzy-1)/2);

        return fuzzy;
    }

    public void distortGrid()
    {
        int x,y,a,an,b,bn;
        int xg, yg;

        an=bn=0;
        xg=yg=0;

        for (x=0; x<gridPoints; x++)
        {
            for (y=0; y<gridPoints; y++)
            {
                a = GridX[x][y];
                an = a + (int)(Math.random()*fuzzy)-hfuzzy;
                if (Math.abs(an-xg)>gridBoxesHalfSize) an=a;
                if ((an<=0)||(an>=255)) an=a;
                GridX[x][y] = an;

                b = GridY[x][y];
                bn = b + (int)(Math.random()*fuzzy)-hfuzzy;
                if (Math.abs(bn-yg)>gridBoxesHalfSize) bn=b;
                if ((bn<=0)||(bn>=255)) bn=b;
                GridY[x][y] = bn;

                yg+=gridBoxesSize;
            }
            xg+=gridBoxesSize;
            yg=0;
        }
    }

    public void blockWarp(int Dest[], int Source[], int Gx, int Gy)
    {
        float VLDx, VRDx, HDx;
        float VLDy, VRDy, HDy;
        float tx1, tx2, ty1, ty2, tx, ty;

        int x,y;
        int Ax, Ay, Bx, By, Cx, Cy, Dx, Dy, Sx, Sy;

        Sx = Gx * gridBoxesSize;
        Sy = Gy * gridBoxesSize;

        VLDx = (GridX[Gx][Gy+1]     -   GridX[Gx][Gy])      / (float)gridBoxesSize;
        VRDx = (GridX[Gx+1][Gy+1]   -   GridX[Gx+1][Gy])    / (float)gridBoxesSize;
        VLDy = (GridY[Gx][Gy+1]     -   GridY[Gx][Gy])      / (float)gridBoxesSize;
        VRDy = (GridY[Gx+1][Gy+1]   -   GridY[Gx+1][Gy])    / (float)gridBoxesSize;

        tx1 = GridX[Gx][Gy];
        ty1 = GridY[Gx][Gy];
        tx2 = GridX[Gx+1][Gy];
        ty2 = GridY[Gx+1][Gy];

        for (y=0; y<gridBoxesSize; y++)
        {
            HDx  = (tx2-tx1) / (float)gridBoxesSize;
            HDy  = (ty2-ty1) / (float)gridBoxesSize;
            tx = tx1;
            ty = ty1;

            for (x=0; x<gridBoxesSize; x++)
            {
                /*
                if (tx>255) tx=255;
                if (ty>255) ty=255;
                if (tx<0) tx=0;
                if (ty<0) ty=0;
                */
                Dest[(Sx+x)+((Sy+y)<<8)] = Source[(int)tx+((int)(ty)<<8)];
                tx += HDx;
                ty += HDy;
            }

            tx1 += VLDx;
            ty1 += VLDy;
            tx2 += VRDx;
            ty2 += VRDy;
         }
    }

    public void feedbackWarp(int Dest[], int Source[])
    {
        for (int i=0; i<65536; i++)
        {
            Dest[i]=Source[i];
        }
    }

    public void feedbackWarpBlur(int Dest[], int Source[])
    {
        int color,rs,gs,bs,rd,gd,bd;
        for (int i=0; i<65536; i++)
        {
            color = Source[i];
            rs = (color & 0x00ff0000) >> 16;
            gs = (color & 0x0000ff00) >> 8;
            bs = color & 0x000000ff;

            color = Dest[i];
            rd = (color & 0x00ff0000) >> 16;
            gd = (color & 0x0000ff00) >> 8;
            bd = color & 0x000000ff;

            rd = (rs + rd) >> 1;
            gd = (gs + gd) >> 1;
            bd = (bs + bd) >> 1;

            Dest[i]=0xff000000 | (rd << 16) | (gd << 8) | bd;
        }
    }
}
