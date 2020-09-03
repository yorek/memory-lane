/* HELL! */
/* All code by Grizly of Rolling Pixels. Some idea by Paul H. Kahler,
how wrote code using PASCAL. I converted his code in C and ASM reaching the
screaming velocity of 42 fps on a 486DX2 66MHz */

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

// Questo serve per usare le istruzione asm a 32 bit!
#pragma inline

// Maschere goniometriche
#define kAngle360				256
#define kAngleMask			(kAngle360 - 1)
#define kAngle720				(kAngle360 * 2)
#define kAngle180				(kAngle360 / 2)
#define kAngle90				(kAngle360 / 4)
#define kPi						3.141592654

// PROTOTIPI *****************************************************************
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
void clear_dest_palette(char c);
void bline(unsigned char far *p,int x1, int y1, int x2, int y2, unsigned char scol, unsigned char ecol);
void loadrps(char *filename, long imgsize, unsigned char far *buf);
void showimage(char *filename);

void DoRotoZoomA(unsigned int x, unsigned int y, unsigned int scale, unsigned char rot);
void DoRotoZoomB(unsigned int x, unsigned int y, unsigned int scale, unsigned char rot);
void DoCopy(unsigned int y);

void SetTables(void);
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
int sinTable[kAngle360],cosTable[kAngle360];
int sin2Table[kAngle360],cos2Table[kAngle360];

// Puntatore VGA
unsigned char far *vga=MK_FP(0xa000,0);

unsigned char far *Buf;
unsigned char huge Map[0xffff];
unsigned char huge Map2[0xffff];
unsigned char huge Map3[0xffff];
// ***************************************************************************

void main(void)
{
	unsigned int rot,rot2,dr,dr2,x,y,dd,dd2,dist,dist2,nc;
	long inizio,fine;

	unsigned char flag,ext;

	Buf=farmalloc(0xffff);
	if (Buf==NULL) exit(1);
	_fmemset(Buf,NULL,0xffff);

	// Setta 320x200
	Set320();

	// Pulisci la memoria video
	WaitVR();
	_fmemset(vga,NULL,0xffff);
	clear_dest_palette(0);

	// Inizializza le tabelle virtuali
	SetTables();

	// Carica le immagini
	loadrps("logo.rps",0xffff,Map);
	loadrps("image.rps",0xffff,Map2);
	loadrps("present.rps",0xffff,Map3);

	// Traccia 2 linee di sepazione.
	bline(Buf,0,50,319,50,15,15);
	bline(Buf,0,149,319,149,15,15);

	// Inizializza l'effetto
	x=128 * 256;
	y=0;

	rot=0;
	dr=1;
	dist=3;
	dd=1;

	rot2=90;
	dr2=1;
	dist2=1200;
	dd2=-1;

	nc=0;
	randomize();
	inizio=biostime(0,0);

	ext=0;flag=1;
	do {
		if ((ext==0) && (flag!=0))
			flag=update_fade_in();

		// x,y	: Centro dell'immagine da ruotare
		// dist	: Distanza dallo schermo. 256 -> 1:1
		// rot	: Angolo di rotazione
		DoRotoZoomA(x,y,dist,rot);
		DoRotoZoomB(x,y,dist2,rot2);
		DoCopy(y);

		// Copia velocemente (per DWORD) Buf in Vga aspettando il retrace
		asm {
			push ES
			push DS

			// in DS:SI il Buffer
			lds si,[Buf]

			// in ES:DI la Vga
			mov ax,0xA000
			mov es,ax
			xor di,di

			// Numero di cicli: 16000 perchŠ copio DWORD
			mov cx,0x3E80

			mov dx,0x03da
		}
wait_ret:
		asm {
			in al,dx
			test al,0x08
			jnz wait_ret

			rep movsd
			pop DS
			pop ES
		}

		rot+=dr;
		dist+=dd;

		rot2+=dr2;
		dist2+=dd2;

		if ((dist==1000) || (dist==2)) dd=-dd;
		if (random(150)==1) dr=random(4)-1;

		if ((dist2==2000) || (dist2==2)) dd2=-dd2;
		if (random(200)==1) dr2=random(5)-2;

		y+=128;
		nc++;

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
	} while (ext!=2);
	fine=biostime(0,0);
	getch();

	// Setta modalit… testo
	SetText();
	farfree(Buf);

	printf("Numero cicli eseguiti: %d.\nTempo impegato: %f sec.\n",nc,((fine-inizio)*10)/182.0);
	printf("Pari a %3.4f frame al secondo!\n",(float)((float)nc/(float)(((fine-inizio)*10)/182.0) ) );

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
// Questa routine deve essere SEMPRE chiamata dopo che un Fade Out Š stato
// completto. C=0 se il fade era al nero, C=63 se era al bianco.
// Le routine Fade_to_black/white la chiamano automaticamente.
// Ricordarsi di chiamarla se il fade lo si fa manualmente con l'update_...
void clear_dest_palette(char c)
{
	int i;

	for (i=0;i<256;i++) {
		destpal[i].r=c;
		destpal[i].g=c;
		destpal[i].b=c;
	}
}

void Fade_to_black(unsigned char from, unsigned char to)
{
	unsigned char flag=0;

	do {
		// se flag=0 allora il fade Š finito
		flag=update_fade_to_black(from,to);
		WaitVR();
	} while(flag==1);

	clear_dest_palette(0);
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

	clear_dest_palette(63);
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
void loadrps(char *filename, long imgsize, unsigned char far *buf)
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
	if (imgsize==0xffff)
		if ((fread(&buf[65535],sizeof(char),1,fp))!=1) {
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

	// Liber la memoria allocata
	farfree(buf);
}

// *************
// ROUNTINE DEMO
// *************
void DoRotoZoomA(unsigned int x, unsigned int y, unsigned int scale, unsigned char rot)
{
	// I valori x , y e scale da passare devono gia essere moltiplicati per 256
	// in quanto i numeri sono trattati nel formato 8:8
	long temp;
	int ddx,ddy,d2x,d2y;
	unsigned int i,j;

	temp=cosTable[rot];
	temp=(temp * scale) >> 8;
	ddx=temp;

	temp=sinTable[rot];
	temp=(temp * scale) >> 8;
	ddy=temp;

	temp=cos2Table[rot];
	temp=(temp * scale) >> 8;
	d2x=temp;

	temp=sin2Table[rot];
	temp=(temp * scale) >> 8;
	d2y=temp;

	i=x - ddx*160 - d2x*100;
	j=y - ddy*160 - d2y*100;

	asm {
		push ES
		push DS

		// Metti il SEG di Map in ES
		mov ax,SEG Map
		mov ES,ax

		// Metti il buffer di destinazione DS:DI
		lds DI,[Buf]
		// Aggiungi il valore all'OFFSET per centrare la schermata
		mov ax,0x3FC0
		add DI,ax

		// Questo serve per velocizzare la rotazione successivamente.
		// E' pi— veloce usare SI che [ddx]
		mov ax,[ddx]
		mov SI,ax

		// Contatore ciclo
		mov cx,98
	}

vloop:
	asm {
		push cx
		// Punto di partenza dal quale incomincia a copiare
		// l'imagine sorgente.
		// Riga
		mov ax,[i]
		// Colonna
		mov dx,[j]

		// Contatore ciclo
		mov cx,64
	}

hloop:
	asm {
		// Ripetendo il codice 5 volte si ottiene un miglioramento del 40 %
		// ***** 1 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		inc DI

		// ***** 2 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		inc DI

		// ***** 3 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		inc DI

		// ***** 4 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		inc DI

		// ***** 5 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		inc DI

		// LOOPPA!
		loop hloop

		// Deforma l'immagine
		mov ax,2
		add si,ax
		add [ddy],ax

		// Passa a copiare la riga successiva
		mov ax,d2x
		mov dx,d2y
		add i,ax
		add j,dx

		// LOOPPA!
		pop cx
		loop vloop

		pop DS
		pop ES
	}
}

void DoRotoZoomB(unsigned int x, unsigned int y, unsigned int scale, unsigned char rot)
{
	// Vedi routine precedente
	long temp;
	int ddx,ddy,d2x,d2y;
	unsigned int i,j;

	temp=cosTable[rot];
	temp=(temp * scale) >> 8;
	ddx=temp;

	temp=sinTable[rot];
	temp=(temp * scale) >> 8;
	ddy=temp;

	temp=cos2Table[rot];
	temp=(temp * scale) >> 8;
	d2x=temp;

	temp=sin2Table[rot];
	temp=(temp * scale) >> 8;
	d2y=temp;

	i=x - ddx*160 - d2x*25;
	j=y - ddy*160 - d2y*25;

	asm {
		push ES
		push DS

		mov ax,SEG Map2
		mov ES,ax

		lds DI,[Buf]

		mov ax,[ddx]
		mov SI,ax

		mov cx,50
	}

vloop:
	asm {
		push cx
		mov ax,[i]
		mov dx,[j]
		mov cx,64
	}

hloop:
	asm {
		// ***** 1 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		mov [DS:DI+0xBB80],bl
		inc DI

		// ***** 2 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		mov [DS:DI+0xBB80],bl
		inc DI

		// ***** 3 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		mov [DS:DI+0xBB80],bl
		inc DI

		// ***** 4 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		mov [DS:DI+0xBB80],bl
		inc DI

		// ***** 5 *****
		add ax,SI
		add dx,[ddy]
		mov bl,ah
		mov bh,dh
		mov bl,[ES:bx]
		mov [DS:DI],bl
		mov [DS:DI+0xBB80],bl
		inc DI

		loop hloop

		mov ax,d2x
		mov dx,d2y
		add i,ax
		add j,dx

		pop cx
		loop vloop

		pop DS
		pop ES
	}
}

void DoCopy(unsigned int y)
{
	asm {
		push ES
		push DS

		mov ax,SEG Map3
		mov ES,ax

		lds DI,[Buf]
		mov ax,0x3FC0
		add DI,ax

		mov dx,[y]
		mov cx,98
	}

vloop:
	asm {
		push cx
		mov ax,0
		mov cx,256
	}

hloop:
	asm {
		// Muoviti a inizio riga
		mov bx,dx

		// Aumenta il valore della riga
		inc al
		mov bl,al

		// Legge il punto nella bitmap fonte
		mov bl,[ES:bx]

		// Trasparenza
		add bl,[DS:DI+32]

		// Visualizza
		mov [DS:DI+32],bl

		inc DI
		loop hloop

		// Finisci tutta la riga vga 320 pixel
		add DI,64

		// Passa alla riga successiva dell'immagine fonte
		add dx,256

		// Riprendi il loop
		pop cx
		loop vloop

		pop DS
		pop ES
	}
}

void SetTables(void)
{
	int dir;
	double angle;

	for (dir=0; dir<kAngle360; dir++) {
		angle=dir;
		angle=angle * kPi / kAngle180;

		sinTable[dir]=(sin(angle)) * 256;
		cosTable[dir]=(cos(angle)) * 256;
		sin2Table[dir]=(sin(angle + kPi / 2)) * 256 * 1.2;
		cos2Table[dir]=(cos(angle + kPi / 2)) * 256 * 1.2;
	}
}

