/****************************************************************************
										            File : PCX.H

 Libreria di lettura da file e scompattazione di file di tipo "pcx".
 Thanks to Ninja.

 Modificata da DeDa il 27-09-1995

 Rimodificata per l'utilizzo di file a qualsiasi risoluzione (256 colori)
 in modalità  protetta a 32 bit (compilatore WATCOM 11)
 da DeDa il 20-08-97
*****************************************************************************/

#ifndef _PCX_LIB_
#define _PCX_LIB_

#include <stdio.h>
#include "gfx32.h"

#define ERRFILE     -1
#define ERRPCX      -2
#define ERRMEM      -3
#define ALLRIGHT     0

// Struttura contenente la chiave di protezione
typedef struct {
    char l1;
    char l2;
    char l3;
} LOCK;

// Struttura contenente l'header del pcx
typedef struct {
    char manufacture;
    char version;
    char encoding;
    char bits_per_pixel;
    short int xmin,ymin;
    short int xmax,ymax;
    short int hres;
    short int vres;
    char palette[48];
    char reserved;
    char colour_planes;
    short int bytes_per_line;
    short int palette_type;
    char filler[58];
} PCXHEAD;


// Header delle funzioni :

/**************************************************************************
 La funzione chiede in ingresso il nome del pcx da scompattare, e due
 puntatori in cui metter… la palette e l'immagine scompattata.
 i Buffer DEVONO essere gia' allocati.
**************************************************************************/
int LeggiPcx (char *nome, BYTE *buffer, RGB *palette);


/**************************************************************************
 Funzione ad uso interno
***************************************************************************/
int ScompattaPcx(FILE *fp, BYTE *buffer);


// Variabili globali

LOCK    rpslock;
PCXHEAD header;
short unsigned width,depth;
short unsigned bytes,bits;


int LeggiPcx (char *nome, BYTE *buffer, RGB *palette)
{
    int i;
    FILE *fp;

    if((fp=fopen(nome,"rb")) == NULL)
		return ERRFILE;
// PER UN USO FUTURO?
/*
    if(fread((char *)&rpslock,1,sizeof(LOCK),fp) != sizeof(LOCK))
		return ERRPCX;
*/
	if(fread((char *)&header,1,sizeof(PCXHEAD),fp) != sizeof(PCXHEAD))
		return ERRPCX;

	if((header.manufacture != 0x0a) || (header.version!=5))
		return ERRPCX;

	if(header.bits_per_pixel==1) bits=header.colour_planes;
		else bits=header.bits_per_pixel;

	if(bits!=8) return ERRPCX;

    if(!fseek(fp,-769L,SEEK_END))
    {
        if(fgetc(fp)!=0x0c || fread(palette,1,768,fp)!=768)
        {
			return ERRPCX;
        } else {
            for ( i=0; i<256; i++ )
            {
                palette[i].r>>=2;
                palette[i].g>>=2;
                palette[i].b>>=2;
            }
        }
    } else return ERRFILE;

	fseek(fp,sizeof(PCXHEAD),SEEK_SET);
	width=(header.xmax-header.xmin)+1;
	depth=(header.ymax-header.ymin)+1;
	bytes=header.bytes_per_line;

	if (ScompattaPcx(fp,buffer)==ERRMEM) {
		fclose(fp);
		return ERRMEM;
	}

	fclose(fp);
    return ALLRIGHT;
 }

int ScompattaPcx(FILE *fp, char *buffer)
{
    int j,k;
    char *wbuf, *wptr, c, l;

    // Alloco e controllo
    wbuf = wptr = (char *)malloc(width);
    if ((wbuf==NULL) || (wptr==NULL)) return ERRMEM;

    // Scompatto
    for (j=0; j<(int)depth; j++) {
        for (k=0; k<(int)width; )
        {
            c = fgetc(fp);
			if (c > 191) {
                l = c - 192;
                c = fgetc(fp);
                memset(wptr, c, l);
                wptr += l;
                k += l;
            } else {
                *wptr = c;
                wptr++;
                k++;
            }
        }
        memcpy(&buffer[(j*width)], wbuf, width);
        wptr = wbuf;
    }

    free(wbuf);
    free(wptr);

    return ALLRIGHT;
}


#endif