/* snwcpyfc */

/* copy-string while folding-case */


/* revision history:

	= 1998-11-01, David A�D� Morano
	This subroutine was originally written.

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

        Ths subroutine is just like 'snwcpy(3dam)' except that it folds the case
        of the characters from the source into the destination.

	int snwcpyfc(dp,dl,sp,sl)
	char		*dp ;
	int		dl ;
	const char	*sp ;
	int		sl ;

	Arguments:

	dp		destination string buffer
	dl		destination string buffer length
	sp		source string
	sl		source string length

	Returns:

	>=0		length of copyied string
	<0		error-overflow


	See-also:

	snwcpy(3dam),
	snwcpylc(3dam),
	snwcpyup(3dam),
	snwcpyfc(3dam),


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpyfc(char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strwcpyfc(char *,const char *,int) ;


/* exported subroutines */


int snwcpyfc(char *dp,int dl,cchar *sp,int sl)
{
	int		rs ;

	if (dl >= 0) {
	    if (sl >= 0) {
	        if (sl > dl) {
	            rs = sncpyfc(dp,dl,sp) ;
	        } else {
	            rs = strwcpyfc(dp,sp,sl) - dp ;
		}
	    } else {
	        rs = sncpyfc(dp,dl,sp) ;
	    }
	} else {
	    rs = strwcpyfc(dp,sp,sl) - dp ;
	}

	return rs ;
}
/* end subroutine (snwcpyfc) */


