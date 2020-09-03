//
// Aspetta il retrace verticale.
//
void WairRet(void);
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

//
// "Sfuoca" lo schermo
//
void ScreenBlend (unsigned char *dest, unsigned char *where);
#pragma aux ScreenBlend =       \
    "add    esi,640"            \
    "add    edi,640"            \
    "mov    ecx,62720"          \
    "xor    ax,ax"              \
    "xor    bx,bx"              \
    "LoopB:"                    \
        "mov    al,[esi]"           \
        "shl    ax,2"               \
        "mov    bl,[esi+640]"       \
        "add    ax,bx"              \
        "mov    bl,[esi+2]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-2]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-640]"       \
        "add    ax,bx"              \
        "shr    ax,3"               \
        "mov    [edi],al"           \
        "inc    edi"                \
        "inc    esi"                \
    "dec    ecx"                \
    "jnz    LoopB"              \
    parm [edi][esi]             \
    modify [eax ebx ecx];

//
// Bilinear filtering
//
void ScreenBlur (unsigned char *dest, unsigned char *src);
#pragma aux ScreenBlur =      \
    "add    esi,320"            \
    "add    edi,320"            \
    "mov    ecx,63360"          \
    "LoopB:"                    \
        "xor    ax,ax"              \
        "xor    bx,bx"              \
        "mov    al,[esi]"           \
        "shl    ax,2"               \
        "mov    bl,[esi+320]"       \
        "add    ax,bx"              \
        "mov    bl,[esi+1]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-1]"         \
        "add    ax,bx"              \
        "mov    bl,[esi-320]"       \
        "add    ax,bx"              \
        "shr    ax,3"               \
        "inc    esi"                \
        "mov    [edi],al"           \
        "inc    edi"                \
    "dec    ecx"                \
    "jnz    LoopB"              \
    parm [edi][esi]             \
    modify [eax ebx ecx];
