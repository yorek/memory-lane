#ifndef TEA_H
#define TEA_H

#define DELTA       0x9E3779B9

/* TEA Prototypes */
void	encipher		(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long);
void	decipher		(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long);
void	encipher_new	(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long);
void	decipher_new	(const unsigned long *v, unsigned long *w, const unsigned long *k, const unsigned long);

#endif
 