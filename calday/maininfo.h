/* maininfo */


/* Copyright � 2004 David A�D� Morano.  All rights reserved. */


#ifndef	MAININFO_INCLUDE
#define	MAININFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<pthread.h>

#include	<vecstr.h>
#include	<localmisc.h>

#include	"sighand.h"


#define	MAININFO	struct maininfo
#define	MAININFO_FL	struct maininfo_flags


struct maininfo_flags {
	uint		progdash:1 ;	/* leading dash on program-name */
	uint		utilout:1 ;	/* utility is out running */
} ;

struct maininfo {
	SIGHAND		sh ;
	vecstr		stores ;
	stack_t		astack ;
	const char	*progdname ;
	const char	*progename ;
	const char	*progname ;
	const char	*srchname ;
	const char	*symname ;
	void		*mdata ;
	MAININFO_FL	have, f, changed, final ;
	MAININFO_FL	open ;
	pthread_t	tid ;
	size_t		msize ;
	volatile int	f_done ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int maininfo_start(MAININFO *,int,const char **) ;
extern int maininfo_setentry(MAININFO *,const char **,const char *,int) ;
extern int maininfo_finish(MAININFO *) ;
extern int maininfo_utilbegin(MAININFO *,int) ;
extern int maininfo_utilend(MAININFO *) ;
extern int maininfo_srchname(MAININFO *,const char **) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAININFO_INCLUDE */


