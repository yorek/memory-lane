// File: RpsDemo.h

#define  SMOOTH_ON      1
#define  SMOOTH_OFF     0

#define kangle360       256
#define kanglemask      (kangle360 - 1)
#define kangle720       (kangle360 * 2)
#define kangle180       (kangle360 / 2)
#define kangle90        (kangle360 / 4)
#define kpi             3.141592654

//
// TYPEDEF
//

typedef unsigned char BYTE;
typedef unsigned int WORD;

typedef struct {
   BYTE r;
   BYTE g;
   BYTE b;
} RGB;

//
// EXTERN
//
extern BYTE *vga;
extern int xlist[200][2];
extern RGB curpal[256];       // Qui si trova la palette corrente
extern RGB destpal[256];      // Qui si trova la palette di destinazione

extern WORD far fnt8[2816];
extern int sine[kangle360];
//
// Prototipi
//

/*-------------------------------------------------------------------------*/
/* SETTA IL MODO VIDEO (0x03; 0x13)                                        */
/*-------------------------------------------------------------------------*/
void VideoMode             (BYTE mode);
#pragma aux VideoMode = \
   "int 0x10"     \
   parm [ax]      \
   modify [ax];

void BufToVga              (BYTE *buf);

void WaitRet               (void);

void VgaToBuf              (BYTE *dest);

void ClearScreen           (BYTE color, BYTE *where);

void Hline                 (WORD x1, WORD x2, WORD y, BYTE color, BYTE *where);

void Bline                 (WORD x1, WORD y1, WORD x2, WORD y2, BYTE color, BYTE *where);

void Draw_Polygon          (int x1, int y1, int x2, int y2, int x3, int y3,
                              int x4, int y4, BYTE color, BYTE *where);

void ScanEdge              (int x1, int y1, int x2, int y2);

void PutPix                (WORD x, WORD y, BYTE numcol, BYTE *Where);

BYTE GetPix                (WORD x, WORD y, BYTE *Where);

void SetColor              (BYTE numcol, BYTE r, BYTE g, BYTE b);

void GetPalette            (void);

void Fade_In               (void);

int update_fade_in         (void);

void Fade_To_Black         (BYTE from, BYTE to);

int update_fade_to_black   (BYTE from, BYTE to);

void Fade_to_white         (BYTE from, BYTE to);

int update_fade_to_white   (BYTE from, BYTE to);

char LoadRps               (char *filename, WORD imgsize, BYTE *buf);

void Stampa                (BYTE *where, char *mex, int y, WORD ct, BYTE smooth);

// Fa lo smooth del colore icol, a seconda dei pixel che gli sono intorno
BYTE Smooth                (WORD x, WORD y, BYTE icol, BYTE *where);

// Fa il Bleed dell'intero buffer
void ScreenBlend           (BYTE *where);

// Indovina?
void DoFire                (BYTE *where);

void CalculateSine         (void);