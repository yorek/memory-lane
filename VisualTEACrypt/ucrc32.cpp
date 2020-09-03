#include <stdio.h>
#include <memory.h>
#include "uCRC32.h"

/*
	Allocate memory and initalize look up table
*/
CRC32::CRC32(void)
{
	crc32_table			= new unsigned long[256];
	crc32_filebuffer	= new unsigned char[1024];
	crc32_init();
	crc = 0xffffffff;
}

/*
	Free memory
*/
CRC32::~CRC32(void)
{		
	delete crc32_table;
	delete crc32_filebuffer;
}

/*
	Swap bit 0 for bit 7, bit 1 for bit 6, etc.
*/
unsigned long CRC32::reflect(unsigned long ref, char ch)
{
	unsigned long value(0);

	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

/*
	Fill the look up table
*/
void CRC32::crc32_init(void)
{
	int i, j;
	
	for(i = 0; i <= 0xFF; i++)
	{
		crc32_table[i] = reflect(i, 8) << 24;
		
		for (j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? CRC32_POLY : 0);

		crc32_table[i] = reflect(crc32_table[i], 32);
	}
}

/*
	Calculate the crc32 of a memory block
*/
unsigned long CRC32::crc32_calc(unsigned char *buf, int len)
{
	unsigned long tcrc;

	tcrc = crc;

	while (len--)
		tcrc = (tcrc >> 8) ^ crc32_table[(tcrc & 0xFF) ^ *buf++];
	
	return tcrc;            
}

/*
	Calculate the crc32 for a file
*/
void CRC32::calculate(FILE *src)
{
	long rcount;

	/* Initialize CRC32 value */
	crc = 0xffffffff;

	/* 
		Read data from source and calculate
		CRC32 incrementally
	*/
	rewind(src);
	while (!feof(src)) 
	{
		memset(crc32_filebuffer, 0, 1024);
		rcount = fread(crc32_filebuffer, sizeof(unsigned char), 1024, src);
		crc = crc32_calc(crc32_filebuffer, rcount); 
	}
	rewind(src);
	
	/* Finalize CRC32 calculation */
	crc = crc^0xffffffff;
}

/*
	Get the crc32 value
*/
unsigned long CRC32::get(void)
{
	return crc;
}







