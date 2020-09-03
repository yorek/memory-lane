#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <alloc.h>
#include <stdlib.h>
#include <time.h>
#include <bios.h>
#include "mess.h"

#define RAW_HEADER_SIZE     17
#define FILE_NOT_FOUND     -10
#define ERROR_READING_FILE -20
#define NO_MEM             -30
#define OK						  0


// questa struttura contiene quattro puntatori ad altrettanti buffer, ci•
// al fine di dividere da subito i punti delle varie pagine
typedef struct {
	unsigned char far *plane_pt[4];
	} BUFFERX;

void SettaModoX (unsigned char modo);
void SettaTesto (void);
void SetVisiblePage (int page, int widthbytes, int height);
void SetVisibleStart (unsigned offset);
void SetActivePage (int page, int widthbytes, int height);
void SetActiveStart (unsigned offset);
void IncActiveStart (unsigned offset);
void VisualizzaImg (BUFFERX buffer, unsigned int startoff, unsigned int dim, long destoff);
void Freexbuffer (BUFFERX buffer);
void WaitVR (void);
int  Allocaxbuffer (BUFFERX *buffer,unsigned dim);

void setcolor(unsigned char numcol, unsigned char r, unsigned char g, unsigned char b);
void firepal(void);

void  putpixel (int x, int y, int widthbytes, char color);
unsigned char  getpixel (int x, int y, int widthbytes);

int Fire (char lac);

// GLOBALS
unsigned visstart=0, actstart=0;
unsigned char far *vga = MK_FP(0xa000,0);

void main (void) {
	SettaModoX(1);
	firepal();
	Fire(0);

	printf("Now interlaced!!!");
	getch();

	SettaModoX(1);
	firepal();
	Fire(1);

	textcolor(14);
	cprintf("X-Mode Fire by GRIZLY and DEDA of Rolling Pixels.\r\n");

	textcolor(9);
	cprintf(linea_1);
	cprintf(linea_2);
	cprintf(linea_3);
	cprintf(linea_4);
	cprintf(linea_5);

	textcolor(12);
	cprintf(linea_6);
}



void SettaModoX (unsigned char modo) {
	union REGS regs;

	// Setto la modalit… VGA standard 13h
	regs.x.ax = 0x0013;
	int86(0x10,&regs,&regs);

	// Resetto il bit CHAIN4-MODE del sequencer
	outpw(0x3c4,0x604);

	// Abilito il Byte-mode sempre del CRTC
	outpw(0x3d4,0xe317);

	// Disabilito il long-mode del CRTC (un registro della VGA)
	outpw(0x3d4,0x0014);

	// Pulisco la memoria video
	outpw(0x3c4,0xf02);
	_fmemset(vga,0,0xffff);

	// Modo 320x400*256
	if (modo) outpw(0x3d4,0x4009);
}


void SetVisiblePage (int page, int widthbytes, int height) {
	SetVisibleStart(page * widthbytes * height);
}


void SetVisibleStart (unsigned offset) {
	visstart=offset;
	outportb(0x3d4,0x0c);              // Setta i byte alti
	outportb(0x3d5,visstart >> 8);
	outportb(0x3d4,0x0d);              // Setta i byte bassi
	outportb(0x3d5,visstart & 0xff);
}


void SetActiveStart (unsigned offset) {
	actstart = offset;
}


void IncActiveStart (unsigned offset) {
	actstart += offset;
}


void SetActivePage (int page, int widthbytes, int height) {
	SetActiveStart(page * widthbytes * height);
}


void putpixel (int x, int y, int widthbytes, char colore) {
	outportb(0x3c4,0x02);
	// Con questo and trova da solo la pagina giusta
	outportb(0x3c5,0x01 << (x&3));
	vga [(unsigned)(widthbytes*y) + (x/4) + actstart] = colore;
}

unsigned char getpixel (int x, int y, int widthbytes) {
	outportb(0x3ce,0x04);
	outportb(0x3cf,(x&3));
	return vga [(unsigned)(widthbytes*y) + (x/4) + actstart];
}

void SettaTesto (void) {
	asm {
		mov ax,3
		int 0x10
	}
}

// Copia dal buffer alla vga in modo X 320x400
// buffer 		Š il buffer (ma va?)
// startoff    Š l'offset del buffer
// dim			Š la dimensione della parte da copiare
// destoff		Š Š l'offset della vga.
void VisualizzaImg (BUFFERX buffer, unsigned int startoff, unsigned int dim, long destoff) {
	unsigned int rdim = dim >> 2;
	unsigned int rdoff = destoff >> 2;
	unsigned int rsoff = startoff >> 2;
	unsigned char i;
	for (i=0;i<4;i++) {
		// Seleziono e riempio i piani
		outportb(0x3c4,0x02);
		outportb(0x3c5,0x01 << i);
		_fmemcpy(vga+rdoff, buffer.plane_pt[i]+rsoff ,rdim );
	}
}

int Allocaxbuffer (BUFFERX *buffer, unsigned int dim) {

	// Alloco i buffers
	if ((buffer->plane_pt[0]=farmalloc(dim/4)) == NULL) return NO_MEM;
	if ((buffer->plane_pt[1]=farmalloc(dim/4)) == NULL) return NO_MEM;
	if ((buffer->plane_pt[2]=farmalloc(dim/4)) == NULL) return NO_MEM;
	if ((buffer->plane_pt[3]=farmalloc(dim/4)) == NULL) return NO_MEM;

	// Li svuoto
	_fmemset(buffer->plane_pt[0],0,dim/4);
	_fmemset(buffer->plane_pt[1],0,dim/4);
	_fmemset(buffer->plane_pt[2],0,dim/4);
	_fmemset(buffer->plane_pt[3],0,dim/4);

	return OK;
}

void Freexbuffer (BUFFERX buffer) {
	int i;
	for (i=0;i<4;i++)
		farfree(buffer.plane_pt[i]);
}


// Setta palette per il fuoco
void firepal(void)
{
	unsigned char i;
	for (i=0;i<64;i++) {
		setcolor(1+i,i/2,0,0);
		setcolor(65+i,32+i/2,i/2,0);
		setcolor(129+i,63,32+i/2,i/2);
		setcolor(192+i,63,63,32+i/2);
	}
}

// Cambia colore
void setcolor(unsigned char numcol, unsigned char r, unsigned char g, unsigned char b)
{
	outportb(0x3c8,numcol);	// Numero colore
	outportb(0x3c9,r); 	 	// Valore di R
	outportb(0x3c9,g);  	  	// Valore di G
	outportb(0x3c9,b);  	  	// Valore di B

}

void WaitVR (void)
{
	unsigned char res;

	do {
		res = inportb(0x03da);}
	while ((res & 8) != 0);

	do {
		res = inportb(0x03da);}
	while ((res & 8) == 0);
}

int Fire (char lac) {
	// Grandezza del fuoco
	int const FW=320;
	int const FH=100;

	unsigned last = FW*(FH-1) >> 2;  // Indirizzo ultima linea

	unsigned char calore;

	int lsx=FW/20;			// limite sinistro del fuoco
	int ldx=9*FW/10;		// limite destro del fuoco

	int temp;
	long fxpos,fypos;
	unsigned int linea,nc;
	unsigned int i,j;
	unsigned char *pa, *pb, *pc, *pd;
	long fireoff=64000+32000*(!lac);

	long inizio,fine;

	BUFFERX flar;

	if (Allocaxbuffer(&flar,32000) != OK) return NO_MEM;

	pa=flar.plane_pt[0];
	pb=flar.plane_pt[1];
	pc=flar.plane_pt[2];
	pd=flar.plane_pt[3];

	nc=0;
	inizio=biostime(0,0);
	do {
		// "Scintille"
		for(i=0;i<35;i++) {
			calore=random(256);
			j=lsx+random(ldx);
			flar.plane_pt[j%4][(j>>2)+last] = calore+random(256-calore);
		}

		linea=FW>>2;
		for (j=linea; j<last; j++) {

			// Calcolo per il primo piano
			temp=((	pa[j+linea] 	+
						pb[j] 			+
						pd[j-1]	 		+
						pa[j]) >> 2);

			temp--;
			temp--;
			pa[j-linea] = temp;
			if (temp < 0 ) pa[j-linea]=0;

			// Calcolo per il secondo piano
			temp=((	pb[j+linea] 	+
						pc[j] 			+
						pa[j] 			+
						pb[j]) >> 2);

			temp--;
			temp--;
			pb[j-linea] = temp;
			if (temp < 0) pb[j-linea]=0;

			// Calcolo per il terzo piano
			temp=((	pc[j+linea] 	+
						pd[j] 			+
						pb[j] 			+
						pc[j]) >> 2);

			temp--;
			temp--;
			pc[j-linea] = temp;
			if (temp < 0) pc[j-linea]=0;

			// Calcolo per il quarto piano
			temp=((	pd[j+linea] 	+
						pa[j+1]			+
						pc[j] 			+
						pd[j]) >> 2);

			temp--;
			temp--;
			pd[j-linea] = temp;
			if (temp < 0) pd[j-linea]=0;
		}

		for (j=last;j<last+80;j++) {
			// Primo piano
			if (pa[j] > 1) {
				pa[j]--;
				pa[j-linea]=pa[j];
			}
			if (random(2)==0)
					pa[j]=(pa[j]+pb[j])>>1;
			else 	pa[j]=(pa[j]+pd[j])>>1;

			// Secondo piano
			if (pb[j] > 1) {
				pb[j]--;
				pb[j-linea]=pb[j];
			}
			if (random(2)==0)
					pb[j]=(pb[j]+pc[j])>>1;
			else 	pb[j]=(pb[j]+pa[j])>>1;

			// Terzo piano
			if (pc[j] > 1) {
				pc[j]--;
				pc[j-linea]=pc[j];
			}
			if (random(2)==0)
					pc[j]=(pc[j]+pd[j])>>1;
			else 	pc[j]=(pc[j]+pb[j])>>1;

			// Quarto piano
			if (pd[j] > 1) {
				pd[j]--;
				pd[j-linea]=pd[j];
				}
			if (random(2)==0)
					pd[j]=(pd[j]+pa[j])>>1;
			else 	pd[j]=(pd[j]+pc[j])>>1;

		}

		// Visualizza :
		// usare FH-2 com ultimo valore per nascondere le ultime 2 righe
		fxpos=fypos=0;
		WaitVR();
		for (i=0;i<FH-2;i++) {
			VisualizzaImg(flar,fxpos,FW,fireoff+fypos);
			fxpos+=FW;
			fypos+=(320<<lac);
			}

		nc++;
	  } while ((kbhit()==0) && (nc<1000));
	fine=biostime(0,0);
	Freexbuffer(flar);

	SettaTesto();
	printf("Going at %3.4f fps!\n",(float)((float)nc/(float)(((fine-inizio)*10)/182.0) ) );
	printf("\n\nBase parameter of coparison:\n");
	printf("486DX2 66MHz : 23 fps!!!!\n\n");

	getch();
	return OK;
}