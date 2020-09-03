#include <stdio.h>

#ifndef CRC32_H
#define CRC32_H

#define CRC32_POLY	0x04c11db7

class CRC32
{
private:
	unsigned long	*crc32_table;
	unsigned char	*crc32_filebuffer;
	unsigned long	crc;

	unsigned long	reflect(unsigned long ref, char ch);
	unsigned long	crc32_calc(unsigned char *buf, int len);
	void			crc32_init(void);

public:
	CRC32(void);
	~CRC32(void);

	void			calculate(FILE *src);
	unsigned long	get(void);
};

#endif
