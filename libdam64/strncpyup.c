/* strncpyuc */

/* routine to convert a string to UPPER case */


#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 1998-11-01, David A�D� Morano

	This subroutine was originally written.


*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is like 'strncpy(3c)' (with its non-NUL terminating
	behavior) except that the case of the characters are converted
	as desired.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<char.h>



/* local defines */

#if	defined(CF_CHAR) && (CF_CHAR == 1)
#define	CF_CHAREXTERN	0
#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#else
#define	CF_CHAREXTERN	1
#endif



/* external subroutines */

extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;

#if	CF_CHAREXTERN
extern int	tolc(int) ;
extern int	touc(int) ;
#endif


/* local variables */


/* exported subroutines */


char *strncpyuc(dst,src,n)
char		*dst ;
const char	*src ;
int		n ;
{
	char	*dp ;


	if (n >= 0) {

	    while (n && *src) {
	        *dst++ = touc(*src) ;
	        src += 1 ;
		n -= 1 ;
	    } /* end while */

	    dp = dst ;
	    if (n > 0)
	        memset(dst,0,n) ;

	} else
	    dp = strcpyuc(dst,src) ;

	return dp ;
}
/* end subroutine (strncpyuc) */


