#ifndef MISC_H
#define MISC_H

#define VERSION		0x00000000 + (0x01<<16) + (0x00<<8) + 0x08
#define AUTHOR		"Davide Mauri"
#define APPNAME		"Visual TEACrypt 80x86 version"
#define FILEEXT		".tdf"
#define LABEL		"TCDM\0"

#define	UNKNOWN		0x0
#define CRYPT		0x1
#define DECRYPT		0x2

#define	OLDALG		0x1
#define	NEWALG		0x2

#define VERBOFF		0x0		
#define VERBON		0x1

#define	MAJOR(V)	(V & 0x00FF0000) >> 16
#define	MINOR(V)	(V & 0x0000FF00) >> 8
#define	RELEASE(V)	(V & 0x000000FF)

/* File Header structure */
struct fileheader_t
{
	char			name[5];
	unsigned long	version;
	unsigned long	filelenght;
	unsigned long	crc;
	unsigned char	algver;		/* Algorithm version */
};

/* Global variables */
unsigned long		*data, *cpd;
unsigned long		key[4] = {0,0,0,0};
int					appmode = UNKNOWN;
char				algotype = NEWALG;
char				verbose = VERBOFF;
unsigned long		crc;
unsigned long       cycles;

/* Function pointer */
void	(*procfun)(const unsigned long *, unsigned long *, const unsigned long *, const unsigned long);

#endif

