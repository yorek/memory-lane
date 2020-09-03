// TEACrypt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define VERSION		0x00000000 + (0x01<<16) + (0x00<<8) + 0x08
#define AUTHOR		"Davide Mauri"
#define COPYRIGHT	"The Tiny Encryption Algorithm (TEA) was developed by David Wheeler and\nRoger Needham at the Computer Laboratory of Cambridge University.\nThe variant used was developed by David Wagner\nThanks to Ross N. Williams for his tutorial on CRC32\nThanks to Billy Bryan for his suggestion on optimization.\n"
#define APPNAME		"TEACrypt 80x86 version"
#define FILEEXT		".tdf"
#define LABEL		"TCDM\0"

#define DELTA		0x9E3779B9

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
char				*srcFile=NULL, *dstFile=NULL, *keyFile=NULL;
unsigned long		*data, *cpd;
unsigned long		key[4] = {0,0,0,0};
int					appmode = UNKNOWN;
char				algotype = NEWALG;	
char				verbose = VERBOFF;
unsigned long		cycles = 16, sum_helper, crc;

/* Misc Prototypes */
int		getparams	(int iNum, char* pPar[]);
void	help		(void);
int		processfile	(void);
int		decrypt		(void);
void	errdesc		(int errnum);

/* TEA Prototypes */
void	encipher		(const unsigned long *v, unsigned long *w, const unsigned long *k);
void	decipher		(const unsigned long *v, unsigned long *w, const unsigned long *k);
void	encipher_new	(const unsigned long *v, unsigned long *w, const unsigned long *k);
void	decipher_new	(const unsigned long *v, unsigned long *w, const unsigned long *k);

/* Function pointer */
void	(*procfun)(const unsigned long *,unsigned long *, const unsigned long *);

/* 
	main
*/
int main(int argc, char *argv[])
{
	printf("\n%s v%i.%i.%i ", APPNAME, MAJOR(VERSION), MINOR(VERSION), RELEASE(VERSION));
	printf("by %s on %s %s\n\n", AUTHOR, __DATE__, __TIME__);	
	
	/* Show help */
	if (argc<=3) {
		help();
		return -1;
	}	

	/* Get Params */
	getparams(argc, argv);

	#ifdef _DEBUG
		verbose = VERBON;
	#endif

	/* Check if command has been specified */
	if (appmode == UNKNOWN)
	{
		errdesc(-3);
		return -3;
	}

	/* Check if source file has been specified */
	if (srcFile == NULL)
	{
		errdesc(-1);
		return -1;
	}

	/* Check if dest file has been specified */
	if (dstFile == NULL)
	{
		errdesc(-2);
		return -2;
	}

	/* Show Params */
	printf(" === FILE  INFO === \n\n");
	printf("Source File\t: %s\n", srcFile);
	printf("Dest File\t: %s\n", dstFile);
	printf("Cycles\t\t: %i\n", cycles);
	if (keyFile==NULL)
		printf("Keys\t\t: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n", key[0], key[1], key[2], key[3]);
	else
		printf("Keyfile\t\t: %s\n", keyFile);
	printf("Algorithm\t: type %i \n", algotype);	

	/* Assign function pointer */
	if (algotype == OLDALG)	{
		procfun = encipher;
		if (appmode == DECRYPT) procfun = decipher;
	} else {
		procfun = encipher_new;
		if (appmode == DECRYPT) procfun = decipher_new;
	}

	/* Process file */
	int result = processfile();

	/* Free allocated memory */
	if (data != NULL) free(data);
	if (cpd != NULL) free(cpd);

	if (result < 0) {
		errdesc(result);
		return -1;
	}
			
	return 0;
}

/* 
	processfile
*/
int processfile(void)
{
	/* 
		TEA is capable of processing 64bit at time
		using 2 long word values (32bit each one
		so totallt we have 64bit or 8 bytes)
		This version has been forced to process 
		4 of 64bit packet a time.
		So the packet lenght is 8 long words
	*/
	char packet_lenght=8;

	/* 
		To speed up processing we can buffer data by
		reading more than 1 packet at time (remember
		that a packet is 8 bytes long). 		
		"bufferpak" contains how many packets we're going
		to buffer. "bufferlen" contains how memory
		(expressed in long words) we need
	*/
	long bufferpak = 2048;
	long bufferlen = packet_lenght * bufferpak;
		
	fileheader_t header;
    FILE *src, *dst, *kf;
	long rcount, rtotal, wcount, wtotal, packlen, kcount;
	long flen;
	clock_t tstart, tend;
	float elaps;
	int keylen, step;	

	/* Create a crc32 class */
	CRC32 filecrc32;

	/* We need to keep downwards compatibilty */
	if (algotype == OLDALG) packet_lenght = 2;

	rtotal = wtotal = 0;
	rcount = wcount = kcount = 0;
	packlen = sizeof(unsigned long) * bufferlen;
	keylen = sizeof(unsigned long) * 4;

	data = (unsigned long *)malloc(packlen);
	if (data == NULL) {
		return -200;
	}

	cpd	= (unsigned long *)malloc(packlen);
	if (cpd == NULL) {
		return -201;
	}
	
	if (verbose == VERBON) {
		printf("Packet size\t: %lu bytes \n",	sizeof(unsigned long) * packet_lenght);
		printf("Buffer size\t: %lu bytes (%lu KB, %lu Packets)\n",	packlen,	packlen / 1024, bufferpak);	
		printf("Allocated mem\t: %lu bytes (%lu KB)\n",	packlen*2+1024,	(packlen*2+1024) / 1024);
	}

	/* Open source file and get lenght */
	src=fopen(srcFile, "rb");
	if (src==NULL) return -100;
	fseek(src, 0, SEEK_END);
	flen = ftell(src);
	rewind(src);
	
	/* Open dest file */
	dst=fopen(dstFile, "wb+");
	if (dst==NULL) {
		fclose(src);
		return -101;
	}

	/* Open key file */
	if (keyFile != NULL) {
		kf=fopen(keyFile, "rb");
		if (kf==NULL) return -103;
	}

	printf("\n");
	
	/* Write or Read header and show operating mode */
	if (appmode == CRYPT) 
	{
		printf(" === CRYPTING === \n\n");
		/* Calculate crc of original data */
		printf("- Calculating CRC32\n");
		crc = 0;
		filecrc32.calculate(src);
		crc = filecrc32.get();

		printf("- Writing header\n");
		strcpy(header.name,LABEL);
		header.crc = crc;
		header.filelenght = flen;
		header.version = VERSION;
		header.algver = algotype;
		fwrite(&header, sizeof(header), 1, dst);				
	} else {
		printf(" === DECRYPTING === \n\n");
		printf("- Reading header...\n");
		fread(&header, sizeof(header), 1, src);			
		/* Since we are decrypting we must have the orginal file lenght */
		if (strcmp(header.name,LABEL)!=0) {
			_fcloseall();
			return -104;
		}
		flen = header.filelenght;
		/* Variable needed to speed up decryption */
		sum_helper = DELTA * cycles;
	}

	/* Show header info */
	if (verbose == VERBON) {
		printf("\tLabel\t: %s\n", header.name);
		printf("\tVersion\t: %i.%i.%i\n", MAJOR(header.version), MINOR(header.version), RELEASE(header.version));
		printf("\tCRC32\t: 0x%08x\n", header.crc);
		printf("\tSize\t: %i bytes\n", flen);
		printf("\tAlgor.\t: type %i \n", header.algver);
	}

	/* Check if right algorithm is used */
	if (header.algver != algotype) {
		_fcloseall();
		return -105;
	}
			
	/* Process file */
	printf("- Processing\n");
	tstart=clock();
	while (!feof(src)) 
	{
		/* Read data from source */
		memset(data, 0, packlen);
		rcount = fread(data, sizeof(char), packlen, src);			
		if (ferror(src)) {
			rtotal=-1;
			break;
		}      
		rtotal += rcount;	
		
		/* Read keys from file if specified */
		if (keyFile != NULL) {
			memset(key, 0, keylen);
			kcount = fread(key, sizeof(unsigned long), 4, kf);			
			if (kcount < 4) rewind(kf);
		}

		/* Process data */
		for (step=0; step<bufferlen; step+=packet_lenght)
		{
			(*procfun)(&data[step], &cpd[step], key); 
		}
		

		/* Write the correct number of bytes */
		if (flen>rtotal) 
		{ 
			wcount = fwrite(cpd, sizeof(char), packlen, dst);			
			wtotal += wcount;		
		} else {
			wcount = fwrite(cpd, sizeof(char), packlen-(rtotal-flen), dst);			
			wtotal += wcount;		
			break;
		}	
	}
	tend=clock();

	if (appmode == DECRYPT) 
	{
		/* Calculate crc of original data */
		printf("- Calculating CRC32\n");
		crc = 0;
		filecrc32.calculate(dst);
		crc = filecrc32.get();

		if (verbose == VERBON) {
			printf("\tCRC32\t: 0x%08x\n", crc);
		}
	}

	_fcloseall();

		/* There was an error during file reading */
	if (rtotal<0) return -102;	

	printf("- Finished\n\n");
	printf(" === STATISTICS === \n\n");

	/* Statistics */
	elaps = (float)(tend-tstart) / (float)CLOCKS_PER_SEC;
	printf("Processed\t: %lu bytes\n", wtotal);
	printf("Time taken\t: %0.3f secs\n", elaps);
	printf("Speed\t\t: %0.3f Kb/s", ((float)wtotal / elaps) / 1024.0f);
	printf(" (%0.3f Mb/s)\n", (((float)wtotal / elaps) / 1024.0f) / 1024.f);

	if (crc != header.crc) 
	{
		printf("\nFILE DECRYPTION FAILURE!\n");
		return -900;
	}

	return 0;
}

/* 
	encipher v 1.0
*/
void encipher(const unsigned long *v, unsigned long *w, const unsigned long *k)
{
	register unsigned long y=v[0], z=v[1], sum=0, a=k[0], b=k[1], c=k[2], d=k[3], n;

	n = cycles;

	while(n-->0)
	{
		sum += DELTA;
		y += (z << 4)+a ^ z+sum ^ (z >> 5)+b;
		z += (y << 4)+c ^ y+sum ^ (y >> 5)+d;
	}

	w[0]=y; w[1]=z;
}


/* 
	decipher v 1.0
*/
void decipher(const unsigned long *v, unsigned long *w, const unsigned long *k)
{
	register unsigned long y=v[0], z=v[1], sum, a=k[0], b=k[1], c=k[2], d=k[3], n;

	n = cycles;

    sum = DELTA * n;

	while(n-->0)
    {
		z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d;
		y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b;
		sum -= DELTA;
    }
   
	w[0]=y; w[1]=z;
}


/* 
	encipher v 2.0 (new variant)
*/
void encipher_new(const unsigned long *v, unsigned long *w, const unsigned long *k)
{
	register unsigned long sum=0, n, y1, z1, y2, z2, y3, z3, y4, z4;

	y1 = v[0];
	z1 = v[1];
	y2 = v[2];
	z2 = v[3];
	y3 = v[4];
	z3 = v[5];
	y4 = v[6];
	z4 = v[7];
	n = cycles;

	while(n-->0)
	{
		y1 += (z1 << 4 ^ z1 >> 5) + z1 ^ sum + k[sum & 3];
		y2 += (z2 << 4 ^ z2 >> 5) + z2 ^ sum + k[sum & 3];
		y3 += (z3 << 4 ^ z3 >> 5) + z3 ^ sum + k[sum & 3];
		y4 += (z4 << 4 ^ z4 >> 5) + z4 ^ sum + k[sum & 3];
		sum += DELTA;
		z1 += (y1 << 4 ^ y1 >> 5) + y1 ^ sum + k[sum>>11 & 3];
		z2 += (y2 << 4 ^ y2 >> 5) + y2 ^ sum + k[sum>>11 & 3];
		z3 += (y3 << 4 ^ y3 >> 5) + y3 ^ sum + k[sum>>11 & 3];
		z4 += (y4 << 4 ^ y4 >> 5) + y4 ^ sum + k[sum>>11 & 3];
	}

	w[0]=y1;
	w[1]=z1;
	w[2]=y2;
	w[3]=z2;
	w[4]=y3;
	w[5]=z3;
	w[6]=y4;
	w[7]=z4;
}

/* 
	decipher v 2.0 (new variant)
*/
void decipher_new(const unsigned long *v, unsigned long *w, const unsigned long *k)
{
	register unsigned long sum, n, y1, z1, y2, z2, y3, z3, y4, z4;

	y1 = v[0];
	z1 = v[1];
	y2 = v[2];
	z2 = v[3];
	y3 = v[4];
	z3 = v[5];
	y4 = v[6];
	z4 = v[7];
	n = cycles;
	sum = sum_helper;

	while (n--)
	{
		z1 -= (y1 << 4 ^ y1 >> 5) + y1 ^ sum + k[sum>>11 & 3];
		z2 -= (y2 << 4 ^ y2 >> 5) + y2 ^ sum + k[sum>>11 & 3];
		z3 -= (y3 << 4 ^ y3 >> 5) + y3 ^ sum + k[sum>>11 & 3];
		z4 -= (y4 << 4 ^ y4 >> 5) + y4 ^ sum + k[sum>>11 & 3];
		sum -= DELTA;
		y1 -= (z1 << 4 ^ z1 >> 5) + z1 ^ sum + k[sum & 3];
		y2 -= (z2 << 4 ^ z2 >> 5) + z2 ^ sum + k[sum & 3];
		y3 -= (z3 << 4 ^ z3 >> 5) + z3 ^ sum + k[sum & 3];
		y4 -= (z4 << 4 ^ z4 >> 5) + z4 ^ sum + k[sum & 3];
	}
	w[0]=y1;
	w[1]=z1;
	w[2]=y2;
	w[3]=z2;
	w[4]=y3;
	w[5]=z3;
	w[6]=y4;
	w[7]=z4;	
}


/* 
	errdesc
*/
void errdesc (int errnum)
{
	printf("\n\n ===== ERROR ====== \n\n");
	printf("ERROR CONDITION FOUND: ");
	switch(errnum)
	{
		case -1:	
			{	
				printf("Please specify the file to be processed\n");
				break;
			}
		case -2:	
			{	
				printf("Please specify the output file\n");
				break;
			}
		case -3:	
			{	
				printf("Command must be specified\n");
				break;
			}
		case -100:	
			{
				printf("Source file not found\n");
				break;
			}
		case -101:	
			{
				printf("Cannot create destination file\n");
				break;
			}
		case -102:	
			{
				printf("Error during file reading\n");
				break;
			}
		case -103:	
			{
				printf("Key file not found\n");
				break;
			}
		case -104:	
			{
				printf("Source file is not a valid TEA crypted file\n");
				break;
			}
		case -105:	
			{
				printf("Wrong algorithm version used\n");
				break;
			}
		case -900:	
			{
				printf("Decrypted file doesn\'t have the expected crc. Probably you used a wrong key\n");
				break;
			}
		default:
		printf("Error code unexpected (%i)\n", errnum);
	}
}

/* 
	getparams
*/
int getparams(int iNum, char *pPar[])
{	
	int i, l;

	/* --- COMMANDS --- */

	/* Set application mode */
	if (strlen(pPar[1]) == 1)
	{
		if (pPar[1][0] == 'c' || pPar[1][0] == 'C') {				
			appmode = CRYPT;
		}
		if (pPar[1][0] == 'd' || pPar[1][0] == 'D') {				
			appmode = DECRYPT;
		}
	}


	/* --- SOURCE AND DEST FILES --- */

	/* Read source file */
	l=strlen(pPar[2])+1;
	srcFile=(char *)malloc(l);		
	strcpy(srcFile, pPar[2]);	

	/* Read destination file */
	l=strlen(pPar[3])+1;
	dstFile=(char *)malloc(l);		
	strcpy(dstFile, pPar[3]);	
	


	/* --- PARAMETERS --- */

	for (i=4; i<iNum; i++) {
		//printf("Par #%i: %s\n", i, pPar[i]);

		if (pPar[i][0] == '-') {			
			
			/* Read key values */
			if (pPar[i][1] == 'k' || pPar[i][1] == 'K') {				
				if (pPar[i][2] == '1' && pPar[i][3] == '=') {
					key[0] = atol(&pPar[i][4]);
				}
				if (pPar[i][2] == '2' && pPar[i][3] == '=') {
					key[1] = atol(&pPar[i][4]);
				}
				if (pPar[i][2] == '3' && pPar[i][3] == '=') {
					key[2] = atol(&pPar[i][4]);
				}
				if (pPar[i][2] == '4' && pPar[i][3] == '=') {
					key[3] = atol(&pPar[i][4]);
				}
				if ((pPar[i][2] == 'f' || pPar[i][2] == 'F') && pPar[i][3] == '=') {
					l=strlen(pPar[i])-3;
					keyFile=(char *)malloc(l+1);		
					strcpy(keyFile, &pPar[i][4]);	
				}
			}

			/* Be verbose */
			if (pPar[i][1] == 'v' || pPar[i][1] == 'V') {				
				verbose = VERBON;
			}

			/* Use old algorithm type */
			if (pPar[i][1] == 'o' || pPar[i][1] == 'O') {				
				algotype = OLDALG;
			}

			/* Set cycle number */
			if (pPar[i][1] == 'n' || pPar[i][1] == 'N') {				
				if (pPar[i][2] == '=') {
					cycles = atoi(&pPar[i][3]);
				}
			}
			
		}
	}

	return 1;
}

/* 
	help
*/
void help(void)
{		
	printf("%s\n\n", COPYRIGHT);
	printf("\tteacrypt {commands} {sourcefile} {destfile} [parameters]\n\n");
	
	printf("  Available commands:\n");
	printf("    c\t\tCrypt {sourcefile} to {destfile}\n");
	printf("    d\t\tDecrypt {sourcefile} to {destfile}\n");

	printf("\n");
	printf("  Available parameters:\n");
	printf("    -k{x}={y}\tKeys. If {x} is a number from 1 to 4 you have to specify the\n");
	printf("    \t\t4 {y} keys manually. Each keys has to be a 32bit integer value.\n");
	printf("    \t\tIf you use 'kf' you have to specify a file which will be used\n");
	printf("    \t\tas key.\n");
	printf("    -v\t\tBe verbose\n");
	printf("    -o\t\tUse original de-encryption algorithm, which is a (very) little\n");
	printf("    \t\tless secure\n");
	printf("    -n={x}\tNumber of de-encryption cycles.\n");
	printf("    \t\t{x} can be any number from 0 up to 255 \n");
	printf("    \t\tDefault is {x}=16 which is a good compromise between security\n");
	printf("    \t\tand speed.\n");
	printf("    \t\t{x}=32 is for high security, x={0} is for no encryption  \n");

	printf("\n");
	printf("  Crypt Examples:\n");
	printf("    \t\tteacrypt c readme.txt readme.tdf -k1=10 -k2=20 -k3=30 -k4=40  \n");
	printf("    \t\tteacrypt c readme.txt readme.tdf -kf=key.jpg  \n");

	printf("\n");
	printf("  Decrypt Examples:\n");
	printf("    \t\tteacrypt d readme.tdf readme_dec.txt -k1=10 -k2=20 -k3=30 -k4=40  \n");
	printf("    \t\tteacrypt d readme.tdf readme_dec.txt -kf=key.jpg  \n");

	printf("\n");
	printf("More information on TEA can be found at:\n\n");
	printf("\thttp://vader.brad.ac.uk/tea/tea.shtml\n");

}
