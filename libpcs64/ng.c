/* ng */

/* search (and other things) a newsgroup list for a newsgroup name */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1


/* revision history:

	= 1995-05-01, David A�D� Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

	This little object just keeps track of a list of newsgroup
	names.


*******************************************************************************/


#define	NG_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"ng.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;


/* external variables */


/* local structures */


/* forward references */

int		ng_add(NG *,const char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int ng_start(NG *ngp)
{
	int	rs ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	rs = vecitem_start(ngp,10,0) ;

	return rs ;
}
/* end subroutine (ng_start) */


int ng_finish(NG *ngp)
{
	NG_ENT	*ep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("mg_finish: non-NULL object\n") ;
#endif

	for (i = 0 ; vecitem_get(ngp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("mg_finish: got a live one\n") ;
#endif

	    if (ep->name != NULL) {
	        rs1 = uc_free(ep->name) ;
		if (rs >= 0) rs = rs1 ;
	    }

	    if (ep->dir != NULL) {
	        rs1 = uc_free(ep->dir) ;
		if (rs >= 0) rs = rs1 ;
	    }

	} /* end for */

#if	CF_DEBUGS
	debugprintf("mg_finish: about to final free\n") ;
#endif

	rs1 = vecitem_finish(ngp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mg_finish) */


int ng_search(ngp,name,rpp)
NG		*ngp ;
const char	name[] ;
NG_ENT	**rpp ;
{
	NG_ENT	*ep ;

	int	rs ;
	int	i ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("ng_search: ent\n") ;
#endif

	if (rpp == NULL)
	    rpp = &ep ;

	for (i = 0 ; (rs = vecitem_get(ngp,i,rpp)) >= 0 ; i += 1) {
	    if ((*rpp) == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("ng_search: loop %i\n",i) ;
#endif

	    if (strcasecmp(name,(*rpp)->name) == 0)
	        break ;

	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (ng_search) */


int ng_add(ngp,ngbuf,nglen,ngdname)
NG		*ngp ;
const char	ngbuf[] ;
int		nglen ;
const char	ngdname[] ;
{
	int	rs ;

	const char	*cp ;

#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	if ((rs = uc_mallocstrw(ngbuf,nglen,&cp)) >= 0) {
	    NG_ENT	ne ;
	    memset(&ne,0,sizeof(NG_ENT)) ;
	    ne.dir = NULL ;
	    ne.len = (rs-1) ;
	    ne.name = cp ;
	    if (ngdname != NULL) {
		const char	*dp ;
		if ((rs = uc_mallocstrw(ngdname,-1,&dp)) >= 0) {
		    ne.dir = dp ;
		}
	    } /* end if (had directory) */
	    if (rs >= 0) {
	        rs = vecitem_add(ngp,&ne,sizeof(NG_ENT)) ;
	    }
	    if (rs < 0) {
		if (ne.dir != NULL) uc_free(ne.dir) ;
		uc_free(cp) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (ng_add) */


int ng_copy(ngp1,ngp2)
NG		*ngp1, *ngp2 ;
{
	NG_ENT	*ep ;

	int	rs = SR_OK ;
	int	i ;
	int	count = 0 ;


#if	CF_SAFE
	if ((ngp1 == NULL) || (ngp2 == NULL))
	    return SR_FAULT ;
#endif

	for (i = 0 ; vecitem_get(ngp2,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    count += 1 ;
	    rs = ng_add(ngp1,ep->name,ep->len,ep->dir) ;

	    if (rs < 0) break ;
	} /* end if */

	return (rs >= 0) ? count : rs ;
}
/* end subroutine (ng_copy) */


int ng_count(ngp)
NG		*ngp ;
{
	int	rs ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	rs = vecitem_count(ngp) ;

	return rs ;
}
/* end subroutine (ng_count) */


int ng_get(ngp,i,rpp)
NG		*ngp ;
int		i ;
NG_ENT	**rpp ;
{
	int	rs ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	rs = vecitem_get(ngp,i,rpp) ;

	return rs ;
}
/* end subroutine (ng_get) */


/* extract newsgroup names from the "newsgroups" header string */
int ng_addparse(ngp,sp,sl)
NG		*ngp ;
const char	sp[] ;
int		sl ;
{
	EMA	aid ;
	EMA_ENT	*ep ;

	int	rs ;
	int	n = 0 ;

#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("ng_addparse: ent\n") ;
	debugprintf("ng_addparse: > %t\n",sp,sl) ;
#endif

	if ((rs = ema_start(&aid)) >= 0) {
	    if ((rs = ema_parse(&aid,sp,sl)) > 0) {
		int	i ;

#if	CF_DEBUGS
	        debugprintf("ng_addparse: got some ema\n") ;
#endif

	        for (i = 0 ; ema_get(&aid,i,&ep) >= 0 ; i += 1) {
	            int		sl = 0 ;
	            const char	*sp = NULL ;
	            if (ep == NULL) continue ;
	            if (ep->f.error) continue ;

	            if ((ep->rp != NULL) && (ep->rl > 0)) {
	                sp = ep->rp ;
	                sl = ep->rl ;
	            } else if ((ep->ap != NULL) && (ep->al > 0)) {
	                sp = ep->ap ;
	                sl = ep->al ;
	            }

		    if (sp != NULL) {
			int		cl ;
			const char	*cp ;
	                if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	                    rs = ng_add(ngp,cp,cl,NULL) ;
			    if (rs < INT_MAX) n += 1 ;
			}
	            } /* end if (had something) */

	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (parse) */
	    ema_finish(&aid) ;
	} /* end if (ema) */

#if	CF_DEBUGS
	debugprintf("ng_addparse: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ng_addparse) */


int ng_parse(ngp,sp,sl)
NG		*ngp ;
const char	sp[] ;
int		sl ;
{
	int	rs ;


#if	CF_SAFE
	if (ngp == NULL)
	    return SR_FAULT ;
#endif

	rs = ng_addparse(ngp,sp,sl) ;

	return rs ;
}
/* end subroutine (ng_parse) */

