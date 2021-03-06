/* u_mincore */

/* get residency status of memory pages */
/* translation layer interface for UNIX� equivalents */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A�D� Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	60
#define	TO_BUSY		60


/* external subroutines */

extern int	msleep(int) ;


/* exported subroutines */


int u_mincore(caddr_t addr,size_t size,char *vec)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		to_busy = TO_BUSY ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_mincore: ent\n") ;
#endif

	repeat {
	    if ((rs = mincore(addr,size,vec)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_BUSY:
	            if (to_busy-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_mincore) */


