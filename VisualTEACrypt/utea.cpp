#include "uTEA.h"

/*
	encipher v 1.0
*/
void encipher(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long cycles)
{
	unsigned long y=v[0], z=v[1], sum=0, a=k[0], b=k[1], c=k[2], d=k[3], n;

	n = cycles;

	while(n-->0)
	{
		sum += DELTA;
		y += (z << 4)+ a ^ z+sum ^ (z >> 5)+b;
		z += (y << 4)+ c ^ y+sum ^ (y >> 5)+d;
	}

	w[0]=y; w[1]=z;
}


/* 
	decipher v 1.0
*/
void decipher(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long cycles)
{
	unsigned long y=v[0], z=v[1], sum, a=k[0], b=k[1], c=k[2], d=k[3], n;

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
void encipher_new(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long cycles)
{
	unsigned long sum=0, n, y1, z1, y2, z2, y3, z3, y4, z4;

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
void decipher_new(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long cycles)
{
	unsigned long sum, n, y1, z1, y2, z2, y3, z3, y4, z4;

	y1 = v[0];
	z1 = v[1];
	y2 = v[2];
	z2 = v[3];
	y3 = v[4];
	z3 = v[5];
	y4 = v[6];
	z4 = v[7];
	n = cycles;
	sum = DELTA * cycles;

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
