/*
	Applet Name	:	WarpFeedback.class
	Version		:   1.1

	Author		: 	Davide Mauri
	E-Mail		: 	davide.mauri@doing.it

    Notes       :   This applet was done after reading the fantastic
                    tutorial on Warp on http://freespace.virgin.net/hugo.elias/

    -----------------------------------------------------------------------------------
		This applet is free of charge only if you NOT use it fo commercial purposes.
    -----------------------------------------------------------------------------------

 	Applet Parameters:
 		Image =
            Name of the image file to warp

        GridSize =
            Subdivide the image into a grid of GridSize x GridSize elements

        MeltSize =
            Define how big will be deformation for each grid box.
                1   means no deformation
               -1   means that the value is calculated in automatic
                3   is what i like
*/


import java.applet.*;
import java.awt.*;
import java.awt.image.*;
import java.math.*;
import dmaEngine.dmaWarp;

public class WarpFeedback extends Applet implements Runnable
{
	/* Thread */
	private Thread dmaThread 		= null;
	private boolean initialized		= false;

    /* Effect started or not */
	private boolean started		    = false;

	/* The double buffer */
	private Image TargetImage=null;
	private int TargetPixel[];

    /* Image to be distorted */
    private Image SourceImage=null;
    private int SourcePixel[];
    private String SourceFile = new String();

    /* Warp Engine */
    private dmaWarp Warp;
    private int meltsize_desired, meltsize_obtained;
    private int granularity;

	/* Guess? */
	private MemoryImageSource dma_mis;
	private ColorModel dma_cm = ColorModel.getRGBdefault();

    /* Applet Size */
    private int dma_width;
    private int dma_height;

    /* FPS */
    private long msStart, msNow, msDiff, msDiff2, lFrames=0;

	/* Alpha Mask */
	final static int a=0xff000000;

	public void initialize()
	{
        /* Initialize applet parameters */
        getAppletSize();
		getParameters();
		initBuffers();
        loadImage();

        /* Initalize Warp Engine */
        Warp = new dmaWarp(granularity);
        meltsize_obtained = Warp.MeltSize(meltsize_desired);

		//Set up the memory image source
		dma_mis=new MemoryImageSource(dma_width,dma_height,dma_cm,TargetPixel,0,dma_width);
		dma_mis.setFullBufferUpdates(true);
		TargetImage=createImage(dma_mis);

		//Garbage collection
		System.gc();
		initialized=true;
	}

    private void getAppletSize()
    {
        dma_width = 256;
        dma_height = 256;
    }

	private void getParameters()
	{
        SourceFile = getParameter("Image");

        /* -1 means that meltsize will be calculated automatically */
        meltsize_desired=-1;
        if (getParameter("MeltStep")!=null) meltsize_desired = Integer.parseInt(getParameter("MeltStep"));

        granularity=16;
        if (getParameter("GridSize")!=null) granularity = Integer.parseInt(getParameter("GridSize"));
	}

	private void initBuffers()
	{
		TargetPixel=new int[dma_width*dma_height];
        SourcePixel=new int[dma_width*dma_height];
	}

	public void start()
	{
		if (dmaThread == null)
		{
			dmaThread = new Thread(this);
			dmaThread.start();
		}
	}

	public void stop()
	{
		if (dmaThread != null)
		{
			dmaThread.stop();
			dmaThread = null;
		}
	}

	public void run()
	{
		while(true)
		{
			repaint();
			try
			{
				dmaThread.sleep(1);
			}
			catch (InterruptedException e)
			{
				System.out.println("dma://interrupted");
			}
		}
	}

	public void paint(Graphics gx)
	{
		gx.setColor(Color.black);
		gx.fillRect(0,0,this.getSize().width,this.getSize().height);
		gx.setColor(new Color(90,90,255));
		gx.drawString("WarpFeedback 1.1",20,20);
		gx.drawLine(0,26,this.getSize().width,26);
		gx.drawLine(10,0,10,this.getSize().height);
		gx.drawString("©2000 by",20,42);
		gx.drawString("Davide Mauri",20,62);
		gx.drawString("Please wait",20,92);
		gx.drawString("while initializing...",20,102);
	}

	public void update(Graphics g)
	{
        int x,y;

		if (initialized)
		{
            clearDoubleBuffer();
            for (y=0; y<granularity; y++)
            {
                for (x=0; x<granularity; x++)
                {
                    Warp.blockWarp(
                                    TargetPixel,
                                    SourcePixel,
                                    x,y
                                );
                }
            }

            if (started)
            {
                Warp.distortGrid();
                Warp.feedbackWarpBlur(SourcePixel, TargetPixel);
            }

            lFrames++;
			TargetImage.flush();
			g.drawImage(TargetImage,0,0,this);

            msNow = System.currentTimeMillis();
            msDiff = msNow - msStart;
            showStatus( "FPS: " + (lFrames * 1000 / msDiff) +
                        " MeltStep: " + meltsize_obtained +
                        " GridSize: " + granularity
                        );
		}
		else
		{
			paint(g);
			initialize();
            msStart = System.currentTimeMillis();
		}
	}

	public boolean imageUpdate(Image image, int i1, int j1, int k, int i2, int j2)
	{
		return true;
	}

    private void clearDoubleBuffer()
	{
		for (int i=0; i<dma_width*dma_height; i++)
		{
			TargetPixel[i]=a;
		}
	}

    private int[] getPixels(Image TempImage, int width, int height)
	{
		int TempPixel[]=new int [width*height];

		showStatus("dma://grabbing...");
		PixelGrabber pg= new PixelGrabber(TempImage,0,0,width,height,TempPixel,0,width);
		try
		{
			pg.grabPixels();
		}
		catch (InterruptedException e)
		{
		}

		return TempPixel;
	}

    private void loadImage()
    {
        showStatus("dma://loading image...");
        SourceImage=getImage(getDocumentBase(),SourceFile);
        SourcePixel=getPixels(SourceImage,dma_width,dma_height);
        SourceImage=null;
    }

    public boolean mouseDown(Event evt, int x, int y)
	{
		started = true;
		return true;
	}
}

