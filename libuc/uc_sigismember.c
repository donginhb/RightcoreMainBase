/* uc_sigismember */

/* interface component for UNIX� library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A�D� Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright � 2000 David A�D� Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_sigismember(sigset_t *sp,int sig)
{
	int		rs ;

	if ((rs = sigismember(sp,sig)) < 0) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_sigismember) */


