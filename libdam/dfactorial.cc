/* dfactorial */

/* floating-pint factorial function */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2004-10-09, David A­D­ Morano
	This was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We calculate the (floating) factorial of the given number.

	Synopsis:

	double dfactorial(int n)

	Arguments:

	n	number to return factorial value for

	Returns:

	-	the factorial number of the input


	Notes:

	The original Fibonacci function:

	double factorial(int n)
	{
	    return (n * dfactorial(n-1)) ;
	}

	Floating-point:

	f(n) = floor(1.0 / sqrt(5.0)) * pow(((1.0 + sqrt(5.0)) / 2.0),(n+1))
	

*******************************************************************************/


#include	<envstandards.h>
#include	<cmath>
#include	<limits.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

double		binexp(double,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local variables */


/* foward references */

extern "C" int	dfactorial(double,int) ;


/* exported subroutines */


double dfactorial(int n)
{
	constexpr double	phi = ((1.0 + sqrt(5.0))/2.0) ;
	double			v = -1.0 ;
	if (n >= 0) {
	    v = 1.0 ;
	    if (n > 1) {
		v = binexp(phi,(n+1)) + 0.5 ;
	    }
	}
	return v ;
}
/* end subroutine (dfactorial) */


