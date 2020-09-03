/* C routines to demonstate MOD-OBJ */
/* You need a project file called TCMOD.PRJ containing the two lines
	 tcmod.c
	 mod-obj.obj
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

void far moddevice( int *device );
void far modvolume( int vol1, int vol2,int vol3,int vol4);
void far modsetup( char *filenm, int looping, int prot,int mixspeed,
									 int device, int *status);
void far modstop(void);
void far modinit(void);

void main()
{
	int dev,mix,vol,stat;
	char md[41];

	modinit();
	moddevice( &dev );
	if (dev == 255) exit(2);
	if (dev == 0)
	{
		printf("\n* The speaker won't sound very good playing back a module\n");
		printf("* Why not make a simple D/A converter using some resistors\n");
		printf("* as detailed in HARDWARE.DOC - it will only cost a couple\n");
		printf("* of pounds/dollars\n\n\n");
	}
	printf("Enter filename          : ");
	gets(md);
	vol = 255;
	mix = 16000;
	modvolume(vol,vol,vol,vol);
	modsetup( md, 4, 0 ,mix, dev, &stat );
	printf("\n");
	if (stat==1)
					{
						printf("Not a mod!\n");
						exit(1);
					}
	if (stat==2)
					{
						printf("Already playing!\n");
						exit(1);
					}
	if (stat==4)
	{
						printf("Out of memory!\n");
						exit(1);
	}

	for (dev=0;dev<5000;dev++) {
		gotoxy(1,wherey());
		printf("I'm counting to 5000...  %d  ",dev);
	}

	modstop();
	exit(0);
}