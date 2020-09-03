// File: Gfx32.h
#ifndef _GFX_32_
#define _GFX_32_

#define VERSION         "2.0 Beta FOR DEMO"
#define DATE            __DATE__

#define kangle360       256
#define kanglemask      (kangle360 - 1)
#define kangle720       (kangle360 * 2)
#define kangle180       (kangle360 / 2)
#define kangle90        (kangle360 / 4)
#define kpi             3.141592654
#define kpi2            2*kpi

#define FLAT            0
#define SMOOTH          1

#define MCGA            64000

//
// TYPEDEF
//

typedef unsigned char BYTE;
typedef unsigned short int WORD;
// Gia' definita nel MIDAS
// typedef unsigned int DWORD;

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
extern RGB curpal[256];
extern RGB destpal[256];
extern WORD fnt8[2816];
extern int sine[kangle360];
extern int x,y,z;
extern int xt,yt,zt;
extern int zoff;
extern int xsin,xcos;
extern int ysin,ycos;
extern int zsin,zcos;
extern BYTE xangle,yangle,zangle;

//
// Prototipi
//

int random                  (int maxvalue);

void Bline                  (int x1, int y1, int x2, int y2, BYTE color, BYTE *where);

void ScanEdge               (int x1, int y1, int x2, int y2);
void Draw_Polygon           (int x1, int y1, int x2, int y2, int x3, int y3,
                              int x4, int y4, BYTE color, BYTE *where);

void SetColor               (BYTE numcol, BYTE r, BYTE g, BYTE b);

void GetPalette             (void);

void Fade_In                (void);
int update_fade_in          (void);

void Fade_To_Black          (BYTE from, BYTE to);
int update_fade_to_black    (BYTE from, BYTE to);

void Fade_to_white          (BYTE from, BYTE to);
int update_fade_to_white    (BYTE from, BYTE to);

void Stampa                 (BYTE *where, char *mex, int y, BYTE ct, BYTE mode);

BYTE SmoothPix              (WORD x, WORD y, BYTE c, BYTE *where);

void CalculateSine          (void);

void ScreenBlend            (BYTE *where);
void ScreenBlend640         (BYTE *where);

void ScreenBlendUp          (BYTE *where);
void ScreenBlendUp640       (BYTE *where);

void Render_Flame           (BYTE *where);
void Render_Flame640        (BYTE *where);

void VideoMode              (BYTE mode);

void BufToVga               (BYTE *buf);

void VgaToBuf               (BYTE *buf);

void FlipMem                (BYTE *dest, BYTE *src);

void WaitRet                (void);

void ClearScreen            (BYTE color, BYTE *where);

void Hline                  (WORD x1, WORD x2, WORD y, BYTE color, BYTE *where);

void RotatePoint            (void);

void UpdateRotation         (BYTE deltax, BYTE deltay, BYTE deltaz);

void FastTrasp              (BYTE *dest, BYTE *src, int cx);
void FastTrasp640           (BYTE *dest, BYTE *src, int cx);

/*-------------------------------------------------------------------------*/
/* SETTA IL MODO VIDEO (0x03; 0x13)                                        */
/*-------------------------------------------------------------------------*/
#pragma aux VideoMode = \
   "int 0x10"     \
   parm [ax]      \
   modify [ax];

/*-------------------------------------------------------------------------*/
/* FUNZIONE BufToVga                                                       */
/*-------------------------------------------------------------------------*/
#pragma aux BufToVga =          \
   "mov     edi, 0x0a0000"      \
   "mov     dx, 0x03da"         \
   "wait_ret:"                  \
   "in      al,dx"              \
   "test    al,0x08"            \
   "jnz     wait_ret"           \
   "mov     ecx, 16000d"        \
   "rep     movsd"              \
   parm [esi]                   \
   modify [esi edi dx al ecx];

/*-------------------------------------------------------------------------*/
/* FUNZIONE VgaToBuf                                                       */
/*-------------------------------------------------------------------------*/
#pragma aux VgaToBuf =     \
   "mov esi, 0x0a0000"     \
   "mov ecx, 16000d"       \
   "rep movsd"             \
   parm [edi]              \
   modify [esi edi ecx];

/*-------------------------------------------------------------------------*/
/* FUNZIONE FlipMem                                                       */
/*-------------------------------------------------------------------------*/
#pragma aux FlipMem =       \
    "mov ecx, 16000d"       \
    "rep movsd"             \
    parm [edi] [esi]        \
    modify [esi edi ecx];

/*-------------------------------------------------------------------------*/
/* FUNIONE WaitRet                                                         */
/*-------------------------------------------------------------------------*/
#pragma aux WaitRet =    \
   "mov dx, 0x03da"      \
   "@loop1:"             \
   "in  al, dx"          \
   "and al, 0x08"        \
   "jnz @loop1"          \
   "@loop2:"             \
   "in  al, dx"          \
   "and al, 0x08"        \
   "jz @loop2"           \
   modify [dx al];

/*-------------------------------------------------------------------------*/
/* FUNZIONE ClearScreen                                                    */
/*-------------------------------------------------------------------------*/
#pragma aux ClearScreen = \
   "mov ah,al"            \
   "push ax"              \
   "shl eax,16"           \
   "pop ax"               \
   "mov ecx, 16000d"      \
   "rep stosd"            \
   parm [al] [edi]        \
   modify [eax ecx edi];

/*-------------------------------------------------------------------------*/
/* FUNZIONE HLINE                                                          */
/*-------------------------------------------------------------------------*/
#pragma aux Hline =     \
    "cmp    ebx,0"      \
    "jge    X1Ok"       \
    "mov    ebx,0"      \
    "X1Ok:"             \
    "cmp    ecx,319"    \
    "jle    X2Ok"       \
    "mov    ecx,319"    \
    "X2Ok:"             \
    "cmp    ebx,ecx"    \
    "jg     Exit"       \
    "push   ax"         \
    "mov    eax,edx"    \
    "shl    eax,8"      \
    "shl    edx,6"      \
    "add    edi,eax"    \
    "add    edi,edx"    \
    "add    edi,ebx"    \
    "pop    ax"         \
    "mov    ah,al"      \
    "sub    ecx,ebx"    \
    "shr    ecx,1"      \
    "jnc    Finish"     \
    "stosb"             \
    "Finish:"           \
    "rep    stosw"      \
    "Exit:"             \
    parm [ebx] [ecx] [edx] [al] [edi] \
    modify [eax ebx ecx edx edi]

/*-------------------------------------------------------------------------*/
/* FUNZIONE ScreenBlend                                                    */
/*-------------------------------------------------------------------------*/
#pragma aux ScreenBlend =       \
    "mov    eax,esi"            \
    "mov    edi,eax"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,319"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,31680"          \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-320]"       \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-320]"       \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    "mov    cx,319"             \
"LoopC:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopC"              \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    parm [esi]                  \
    modify [esi edi eax bx cx];

#pragma aux ScreenBlend640 =    \
    "mov    eax,esi"            \
    "mov    edi,eax"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,639"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,152960"          \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "mov    bl,[esi-640]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "mov    bl,[esi-640]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    "mov    cx,639"             \
"LoopC:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-640]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopC"              \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "shl    ax,2"               \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-640]"       \
    "add    ax,bx"              \
    "shr    ax,3"               \
    "mov    [edi],al"           \
    parm [esi]                  \
    modify [esi edi eax bx cx];

/*-------------------------------------------------------------------------*/
/* FUNZIONE ScreenBlendUp                                                         */
/*-------------------------------------------------------------------------*/
#pragma aux ScreenBlendUp=      \
    "mov    edx,esi"            \
    "mov    edi,edx"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,319"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,31680"          \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    "mov    cx,319"             \
"LoopC:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopC"              \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    parm [esi]                  \
    modify [esi edi eax ebx ecx edx];

#pragma aux ScreenBlendUp640=   \
    "mov    edx,esi"            \
    "mov    edi,edx"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,639"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,152960"          \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    "mov    cx,639"             \
"LoopC:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopC"              \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    parm [esi]                  \
    modify [esi edi eax ebx ecx edx];

/*-------------------------------------------------------------------------*/
/* FUNZIONE Render_Flame                                                          */
/*-------------------------------------------------------------------------*/
#pragma aux Render_Flame=       \
    "mov    edx,esi"            \
    "mov    edi,edx"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,319"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,31680"          \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "dec    ax"                 \
    "jns    D1"                 \
    "xor    al,al"              \
"D1:"                           \
    "mov    [edi-320],al"       \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+320]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "dec    ax"                 \
    "jns    D2"                 \
    "xor    al,al"              \
"D2:"                           \
    "mov    [edi-320],al"       \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    parm [esi]                  \
    modify [esi edi eax ebx ecx edx];

#pragma aux Render_Flame640=    \
    "mov    edx,esi"            \
    "mov    edi,edx"            \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "mov    cx,639"             \
"LoopA:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi],al"           \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopA"              \
    "mov    ecx,152960"         \
"LoopB:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi-640],al"       \
    "inc    edi"                \
    "inc    esi"                \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi+640]"       \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi-640],al"       \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopB"              \
    "mov    cx,319"             \
"LoopC:"                        \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi+1]"         \
    "add    ax,bx"              \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi-640],al"       \
    "inc    edi"                \
    "inc    esi"                \
    "loop   LoopC"              \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "mov    al,[esi]"           \
    "mov    bl,[esi-1]"         \
    "add    ax,bx"              \
    "shr    ax,2"               \
    "mov    [edi-640],al"       \
    parm [esi]                  \
    modify [esi edi eax ebx ecx edx];

/*-------------------------------------------------------------------------*/
/* FUNZIONE FastTrasp                                                             */
/*-------------------------------------------------------------------------*/
#pragma aux FastTrasp = \
    "add    edi,ecx"    \
    "mov    cx,200"     \
    "xor    ax,ax"      \
"LoopY:"                \
    "push   cx"         \
    "mov    cx,88"      \
"LoopX:"                \
    "mov    al,[esi]"   \
    "cmp    al,0"       \
    "je     NoDraw"     \
    "add    al,[edi]"   \
    "jnc    Draw"       \
    "mov    al,255"     \
"Draw:"                 \
    "mov    [edi],al"   \
"NoDraw:"               \
    "inc    edi"        \
    "inc    esi"        \
    "loop   LoopX"      \
    "pop    cx"         \
    "add    edi,232"    \
    "loop   LoopY"      \
    parm [edi] [esi] [ecx]   \
    modify [esi edi eax ebx cx];

#pragma aux FastTrasp640 = \
    "add    edi,ecx"    \
    "mov    cx,480"     \
"LoopY:"                \
    "push   cx"         \
    "mov    cx,186"     \
"LoopX:"                \
    "mov    al,[esi]"   \
    "cmp    al,0"       \
    "je     NoDraw"     \
    "add    al,[edi]"   \
    "jnc    Draw"       \
    "mov    al,255"     \
"Draw:"                 \
    "mov    [edi],al"   \
"NoDraw:"               \
    "inc    edi"        \
    "inc    esi"        \
    "loop   LoopX"      \
    "pop    cx"         \
    "add    edi,454"    \
    "loop   LoopY"      \
    parm [edi] [esi] [ecx]   \
    modify [esi edi eax ebx cx];

#endif
