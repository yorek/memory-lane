#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <mem.h>
#include <alloc.h>
#include <dos.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <bios.h>
#include "c:\tc31\rps\demo\mess.h"

#pragma inline

#define MAXSTARS 400

#define kAngle360				256
#define kAngleMask			(kAngle360 - 1)
#define kAngle720				(kAngle360 * 2)
#define kAngle180				(kAngle360 / 2)
#define kAngle90				(kAngle360 / 4)
#define kPi						3.141592654

// PROTOTIPI GENERALI PER QUALSIASI DEMO**************************************
void Set320(void);
void SetText(void);
void WaitVR(void);
void setcolor(unsigned char numcol, unsigned char r, unsigned char g, unsigned char b);
void Fade_to_black(unsigned char from, unsigned char to);
int update_fade_to_black(unsigned char from, unsigned char to);
void Fade_to_white(unsigned char from, unsigned char to);
int update_fade_to_white(unsigned char from, unsigned char to);
void Fade_In(void);
int update_fade_in(void);
void getpalette(void);
void bline(unsigned char far *p,int x1, int y1, int x2, int y2, unsigned char scol, unsigned char ecol);
void loadrps(char *filename, unsigned int imgsize, unsigned char far *buf);
void showimage(char *filename);
void dofire(void);
// ***************************************************************************

// Variabili Globali *********************************************************
// Palette 256 colori . 3 byte per colore (RGB)
typedef struct {
			unsigned char r;
			unsigned char g;
			unsigned char b;
			} RGB;
// La palette attualmente sul video. E' aggiornata ogni volta si fa un FADE
// NON ALTERARE MAI MANUALMENTE!!!!
RGB pal[256];
// La palette di destinazione. Il Fade In copia questa palette in PAL
// Dopo un Fade In PAL e DESTPAL contengo gli stessi valori.
// Tutte le operazioni sulla palette vengono fatte su questa palette
RGB destpal[256];

// Tabelle virtuali sin e cos
long sinTable[kAngle360],cosTable[kAngle360];

// Puntatore VGA
unsigned char far *vga=MK_FP(0xa000,0);

// Esterne
extern unsigned char linea_1[],linea_2[],linea_3[],
							linea_4[],linea_5[],linea_6[];
// ***************************************************************************

void main()
{
	// Setta 320x200
	Set320();

	// Pulisci la memoria video
	WaitVR();
	_fmemset(vga,NULL,0xffff);

	// Messaggio introduttivo
	showimage("Fire.int");
	WaitVR();
	_fmemset(vga,NULL,0xffff);

	// Effetto
	dofire();

	// Setta modalit… testo
	SetText();

	textcolor(14);
	cprintf("Fire by GRIZLY of Rolling Pixels.\r\n");

	textcolor(9);
	cprintf(linea_1);
	cprintf(linea_2);
	cprintf(linea_3);
	cprintf(linea_4);
	cprintf(linea_5);

	textcolor(12);
	cprintf(linea_6);

}

// ******************** //
// ***** FUNZIONI ***** //
// ******************** //
// Setta 320x200x256
void Set320(void)
{
	asm {
		mov ax,0x0013
		int 0x0010
	}
}

// Setta modalit… testo
void SetText(void)
{
	asm {
		mov ax,0x0003
		int 0x0010
	}
}

// Aspetta il ritracciamento verticale
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

// Cambia colore
void setcolor(unsigned char numcol, unsigned char r, unsigned char g, unsigned char b)
{
	outportb(0x3c8,numcol);	// Numero colore
	outportb(0x3c9,r); 	 	// Valore di R
	outportb(0x3c9,g);  	  	// Valore di G
	outportb(0x3c9,b);  	  	// Valore di B
}

// Memorizza la palette
void getpalette(void)
{
	unsigned i;

	// Inizia dal colore 0
	outportb(0x3c7,0x0);
	for (i=0;i<256;i++) {
		pal[i].r=inportb(0x3c9);
		pal[i].g=inportb(0x3c9);
		pal[i].b=inportb(0x3c9);
	}
}

/*****************************************************************************
 ROUTINE PER IL FADING IN
*****************************************************************************/
void Fade_In(void)
{
	unsigned char flag=1;

	do {
		flag=update_fade_in();
		WaitVR();
	} while (flag==1);
}

int update_fade_in(void)
{
	int i;
	unsigned char flag=0;

	outportb(0x3c8,0x0);
	for(i=0;i<256;i++) {
			if (pal[i].r<destpal[i].r)
				pal[i].r++;
			if (pal[i].r>destpal[i].r)
				pal[i].r--;
			outportb(0x3c9,pal[i].r);

			if (pal[i].g<destpal[i].g)
				pal[i].g++;
			if (pal[i].g>destpal[i].g)
				pal[i].g--;
			outportb(0x3c9,pal[i].g);

			if (pal[i].b<destpal[i].b)
				pal[i].b++;
			if (pal[i].b>destpal[i].b)
				pal[i].b--;
			outportb(0x3c9,pal[i].b);

	}
	flag=0;
	for (i=0;i<256;i++) {
		if (pal[i].r != destpal[i].r) {
			flag=1;
			break;
		}
		if (pal[i].g != destpal[i].g) {
			flag=1;
			break;
		}
		if (pal[i].b != destpal[i].b) {
			flag=1;
			break;
		}
	}
	return flag;
}

/*****************************************************************************
 Routine per il FADING OUT al nero e al bianco
*****************************************************************************/
void Fade_to_black(unsigned char from, unsigned char to)
{
	unsigned char flag=0;

	do {
		// se flag=0 allora il fade Š finito
		flag=update_fade_to_black(from,to);
		WaitVR();
	} while(flag==1);
}

int update_fade_to_black(unsigned char from, unsigned char to)
{
	int i;
	unsigned char flag;

	outportb(0x3c8,0x0);
	for (i=from; i<to+1; i++) {
		if (pal[i].r>0) pal[i].r--;
		outportb(0x3c9,pal[i].r);

		if (pal[i].g>0) pal[i].g--;
		outportb(0x3c9,pal[i].g);

		if (pal[i].b>0) pal[i].b--;
		outportb(0x3c9,pal[i].b);
	}

	flag=0;
	for (i=from; i<to+1; i++) {
		if (pal[i].r != 0) {
			flag=1;
			break;
		}
		if (pal[i].g != 0) {
			flag=1;
			break;
		}
		if (pal[i].b != 0) {
			flag=1;
			break;
		}
	}
	return flag;
}

void Fade_to_white(unsigned char from, unsigned char to)
{
	unsigned char flag=0;

	do {
		// se flag=0 allora il fade Š finito
		flag=update_fade_to_white(from,to);
		WaitVR();
	} while(flag==1);
}

int update_fade_to_white(unsigned char from, unsigned char to)
{
	int i;
	unsigned char flag;

	outportb(0x3c8,0x0);
	for (i=from; i<to+1; i++) {
		if (pal[i].r<63) pal[i].r++;
		outportb(0x3c9,pal[i].r);

		if (pal[i].g<63) pal[i].g++;
		outportb(0x3c9,pal[i].g);

		if (pal[i].b<63) pal[i].b++;
		outportb(0x3c9,pal[i].b);
	}

	flag=0;
	for (i=from; i<to+1; i++) {
		if (pal[i].r != 63) {
			flag=1;
			break;
		}
		if (pal[i].g != 63) {
			flag=1;
			break;
		}
		if (pal[i].b != 63) {
			flag=1;
			break;
		}
	}
	return flag;
}


// **************
// DISK FUNCTIONS
// **************
// Carica un file .RPS
// NESSUN OUTPUT SU SCHERMO E' EFFETTUATO IN QUESTA PROCEDURA.
// Il file viene messo in BUF e la palette in DESTPAL.
// Sar… necessario quindi poi copiare il buffer su schermo e quindi
// settare la palette video.
// [imgsize] Š la grandezza dell'imagine ESCLUSI header e tavolozza
// [buf] Š il buffe dove verr… messa l'imagine. DEVE ESSERE GIA ALLOCATO
// E VERIFICATO!!!!!
void loadrps(char *filename, unsigned int imgsize, unsigned char far *buf)
{
	unsigned char r,g,b;
	int i;
	char header[17];
	FILE *fp;

	// Svuota il buffer
	_fmemset(buf,NULL,imgsize);

	// apre il file
	if ((fp=fopen(filename,"rb"))==NULL) {
		SetText();
		printf("Hey...non trovo un file!");
		exit(1);
	}

	// legge l'header dell'immagine
	if ((fread(header,sizeof(char),17,fp))!=17) {
		SetText();
		printf("Errore di lettura dell'header del file!");
		exit(1);
	}

	// legge i dati
	if ((fread(buf,sizeof(char),imgsize,fp))!=imgsize) {
		SetText();
		printf("Errore di lettura dei dati!");
		exit(1);
	}

	// legge e memorizza la palette
	for (i=0;i<256;i++) {
		r=fgetc(fp)>>2;
		g=fgetc(fp)>>2;
		b=fgetc(fp)>>2;
		destpal[i].r=r;
		destpal[i].g=g;
		destpal[i].b=b;
	}

	// chiude il file
	fclose(fp);
}

// ***********
// Routine FX
// ***********
// Traccia una linea nel buffer p.
void bline(unsigned char far *p,int x1, int y1, int x2, int y2, unsigned char scol,unsigned char ecol)
{
	// Per il tracciamento della linea
	int i,deltax,deltay,numpix,d,dinc1,dinc2,x,xinc1,xinc2,y,yinc1,yinc2;

	// Per il colore della linea
	float deltac;

	// in modo che il colore finale sia sempre minore di quello iniziale
	deltac=ecol-scol;

	deltax=abs(x2-x1);
	deltay=abs(y2-y1);

	if (deltax >= deltay) {
		numpix=deltax+1;
		d=(deltay<<1)-deltax;
		dinc1=deltay<<1;
		dinc2=(deltay-deltax)<<1;
		xinc1=1;
		xinc2=1;
		yinc1=0;
		yinc2=1;
	} else {
		numpix=deltay+1;
		d=(deltax<<1)-deltay;
		dinc1=deltax<<1;
		dinc2=(deltax-deltay)<<1;
		xinc1=0;
		xinc2=1;
		yinc1=1;
		yinc2=1;
	}

	if (x1>x2) {
		xinc1 = -xinc1;
		xinc2 = -xinc2;
	}

	if (y1>y2) {
		yinc1 = -yinc1;
		yinc2 = -yinc2;
	}
	x=x1;
	y=y1;

	deltac=(deltac/(numpix+1));

	for (i=0;i<numpix;i++) {
		p[x+(y<<8)+(y<<6)]=scol+(deltac*(i+1));
		if (d<0) {
			d+=dinc1;
			x+=xinc1;
			y+=yinc1;
		} else {
			d+=dinc2;
			x+=xinc2;
			y+=yinc2;
		}
	}
}

// **************************************
// ROUTINE PER LE SCHERMATE DI INTERMEZZO
// **************************************
void showimage(char *filename)
{
	unsigned int i,j,k;

	// Alloca memoria per questa parte del demo
	unsigned char far *buf;

	buf=malloc(0xffff);
	if (buf==NULL) {
		SetText();
		printf("Errore di allocazione di memoria in SHOWIMAGE\n");
	}
	_fmemset(buf,NULL,0xffff);


	// Carica il file .INT che dovr… essere visializzato.
	// I file .INT sono fatti come:
	// 17	 	bytes di header
	// 16000 bytes di dati (1a immagine di 320x50)
	// 16000 bytes di dati (2a immagine di 320x50)
	// 768 	bytes di palette.
	loadrps(filename,32000,buf);

	// Disegna il bordo superiore nel buffer dalla linea 101
	for (j=0;j<16;j++)
		_fmemset(&buf[32320+j*320],j*16,320);

	// Disegna il bordo inferiore nel buffer dalla linea 117
	for (j=0;j<16;j++)
		_fmemset(&buf[37440+j*320],255-j*16,320);

	// Mette i bordi sullo schermo
	WaitVR();
	_fmemcpy(&vga[59*320],&buf[32320],5120);
	_fmemcpy(&vga[125*320],&buf[37440],5120);

	Fade_In();
	sleep(1);

	// Fai apparire la prima scritta
	for (j=0;j<256;j++) {
		// Fai tutte le operazioni nel buffer
		for (i=0;i<16000;i++)
			if (vga[24000+i]<buf[i]) buf[42560+i]++;

		// Ora copia la nuovo immagine (nel buffer alla linea 133) in memoria
		_fmemcpy(&vga[24000],&buf[42560],16000);
	}
	sleep(1);

	// Fai apparire l'altra scritta
	for (j=0;j<256;j++) {
		for (i=0;i<16000;i++) {
			if (vga[24000+i]<buf[16000+i]) buf[42560+i]++;
			if (vga[24000+i]>buf[16000+i]) buf[42560+i]--;
		}

		_fmemcpy(&vga[24000],&buf[42560],16000);
	}

	sleep(2);

	// Fade out e scroll dei bordi
	for (i=58,k=126,j=0; i>0; i--,k++,j++) {
		WaitVR();
		update_fade_to_black(0,255);

		_fmemcpy(&vga[i*320],&buf[32320],5120);
		_fmemcpy(&vga[k*320],&buf[37440],5120);

		bline(vga,0,i+16,319,i+16,0,0);
		bline(vga,0,k-1,319,k-1,0,0);
	}

	// Libera la memoria allocata
	farfree(buf);
}

// ****************
// * ROUTINE DEMO *
// ****************
void setfire1(void)
{
	char i;

	// LA PALETTE DA 128 CONTIENE I VALORI PER IL FUOCO!
	for (i=0;i<32;i++) {
		destpal[(128+i)].r=i;
		destpal[(128+i)].g=0;
		destpal[(128+i)].b=0;

		destpal[(160+i)].r=32+i;
		destpal[(160+i)].g=i;
		destpal[(160+i)].b=0;

		destpal[(192+i)].r=63;
		destpal[(192+i)].g=i+32;
		destpal[(192+i)].b=i;

		destpal[(224+i)].r=63;
		destpal[(224+i)].g=63;
		destpal[(224+i)].b=i+32;
	}
}

void setfire2(void)
{
	char i;

	for (i=0;i<32;i++) {
		destpal[(128+i)].r=0;
		destpal[(128+i)].g=0;
		destpal[(128+i)].b=i;

		destpal[(160+i)].r=0;
		destpal[(160+i)].g=i;
		destpal[(160+i)].b=32+i;

		destpal[(192+i)].r=i;
		destpal[(192+i)].g=i+32;
		destpal[(192+i)].b=63;

		destpal[(224+i)].r=i+32;
		destpal[(224+i)].g=63;
		destpal[(224+i)].b=63;
	}
}

void dofire(void)
{
	unsigned char ext,flag;

	unsigned int ulimit,dlimit,dxl,sxl,ciclo;
	int temp,i,j,bpos;

	int FW=90;
	int FH=90;

	// Alloca memoria per questa parte del demo
	unsigned char far *buf;

	buf=malloc(0xffff);
	if (buf==NULL) {
		SetText();
		printf("Errore di allocazione di memoria in DOFIRE()\n");
	}
	_fmemset(buf,NULL,0xffff);

	loadrps("column.rps",9720,buf);
	for (i=0;i<20;i++) {
			_fmemcpy(&vga[57280+i*320],&buf[i*90],90);
			_fmemcpy(&vga[229+57280+i*320],&buf[i*90],90);
	}

	// Setta la prima palette
	setfire1();

	// Copia la palette dalla memoria la video
	Fade_In();

	// Altra palette per il fuoco
	setfire2();

	// inizializza il buffer
	_fmemset(buf,NULL,0xffff);

	// setta le variabili
	ext=0;
	ciclo=0;
	ulimit=FW*(FH-1);
	dlimit=FW*FH;

	// limite destro
	dxl=9*FW/10;

	// limite sinistro
	sxl=FW/20;

	flag=1;
	do {
		WaitVR();

		for(i=0;i<35;i++) {
			temp=sxl+random(dxl)+ulimit;

			if (ext==0) {
				buf[temp] = 176+random(80);
			} else {
				if (random(9)==0) buf[temp] = 0;
			}
		}

		for (j=FW; j<ulimit; j++) {
			// j+FW= pixel sotto quello attuale
			// j+1= pixel a dx di quello attuale
			// j-1= pixel a sx di quello attuale
			// j-FW=pixel sopra quello attuale
			temp=((buf[j+FW]+buf[j+1]+buf[j-1]+buf[j]) >> 2);
			temp--;
			if (temp<128) temp=128;
			buf[j-FW] = temp;
		}

		for (i=ulimit; i<dlimit; i++)
			if (buf[i]>1) {
				// Velocit… con cui una fiamma si spegne.
				// Pi— Š alto piu Š veloce l'effetto.
				// Ci• perchŠ il nero Š il colore 0
				buf[i]--;
				buf[i-FW]=buf[i];
			}

		// per un effetto ancora pi— reale..Togliere per credere!
		for (i=ulimit; i<dlimit; i++)
			// il valore in random() Š la forza del "vento"
			if (random(2)==0) buf[i]=(buf[i]+buf[i+1])>>1;
			else buf[i]=(buf[i]+buf[i-1])>>1;

		// Visualizza
		// usare FH come valore limite di 'i' per nascondere le ultime 2 righe
		bpos=0;
		for (i=0;i<(FH-2);i++) {
			_fmemcpy(&vga[29440+(i<<8)+(i<<6)],&buf[bpos],FW);
			_fmemcpy(&vga[29440+229+(i<<8)+(i<<6)],&buf[bpos],FW);
			bpos+=FW;
		}

		if ( (kbhit()!=0) && (ext==0) ) {
			flag=1;
			ext=1;
		}

		if (ext==1)
			if (flag!=0) {
				flag=update_fade_to_black(0,255);
			} else {
				ext=2;
			}

		if (ciclo<500) ciclo++;

		if ( (ciclo==500) && (flag!=0) && (ext==0) )
			flag=update_fade_in();

		if ( (ext==0) && (flag==0) && (ciclo==500) ) {
			getpalette();
			ciclo++;
		}

	} while (ext!=2);
	getch();

	// Libera la memoria allocata
	farfree(buf);
}