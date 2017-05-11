/* sha */


#ifndef SHA_H
#define SHA_H	1


/* NIST Secure Hash Algorithm */
/* heavily modified by Uwe Hollerbach <uh@alumni.caltech edu> */
/* from Peter C. Gutmann's implementation as found in */
/* Applied Cryptography by Bruce Schneier */

/* This code is in the public domain */


#ifndef ENDIAN_H
#define ENDIAN_H

/* Warning! this file is automatically generated; changes WILL disappear */

#define SHA_BYTE_ORDER 4321

#define SHA_VERSION 1

#endif /* ENDIAN_H */


/* Useful defines & typedefs */

#define	SHA			SHA_INFO


typedef unsigned char SHA_BYTE;	/* 8-bit quantity */
typedef unsigned long	SHA_LONG;	/* 32-or-more-bit quantity */

#define SHA_BLOCKSIZE		64
#define SHA_DIGESTSIZE		20

typedef struct {
    SHA_LONG digest[5];		/* message digest */
    SHA_LONG count_lo, count_hi;	/* 64-bit bit count */
    SHA_BYTE data[SHA_BLOCKSIZE];	/* SHA data buffer */
    int local;			/* unprocessed amount in data */
} SHA_INFO ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int sha_init(SHA *) ;
extern int sha_update(SHA *,char *, int) ;
extern int sha_digest(SHA *,unsigned char [20]) ;
extern int sha_free(SHA *) ;


#ifdef SHA_FOR_C

#include <stdlib.h>
#include <stdio.h>

void sha_stream(unsigned char [20], SHA_INFO *, FILE *);
void sha_print(unsigned char [20]);
char *sha_version(void);
#endif /* SHA_FOR_C */

#ifdef	__cplusplus
}
#endif


#endif /* SHA_H */

