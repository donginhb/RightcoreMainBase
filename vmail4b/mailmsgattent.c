/* mailmsgattent */

/* extra subroutines for the 'mailmsgattent' object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A�D� Morano
        The subroutine was written from scratch to do what the previous program
        by the same name did.

*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines are extra methods to the MAILMSGATTENT object. See the
        code for details!

	Grammar:
		content_spec := type | { type "/" subtype } | file_extension
		attspec := filename | { content_spec "=" filename }


*******************************************************************************/


#define	MAILMSGATTENT_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<contypevals.h>
#include	<contentencodings.h>
#include	<ismmclass.h>
#include	<localmisc.h>

#include	"mailmsgattent.h"


/* local defines */

#define	MAILMSGATTENT_TMPCNAME	"mkmsgXXXXXXXXX"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sisub(cchar *,int,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(cchar *,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*mallocstrw(cchar *,int) ;
extern cchar	*mallocstr(cchar *) ;


/* external variables */


/* local structures */


/* forward references */

int		mailmsgattent_analyze(MAILMSGATTENT *,const char *) ;
int		mailmsgattent_finish(MAILMSGATTENT *) ;
int		mailmsgattent_isplaintext(MAILMSGATTENT *) ;

static int	mailmsgattent_startfn(MAILMSGATTENT *,cchar *,int) ;
static int	mailmsgattent_startct(MAILMSGATTENT *,cchar *,int) ;
static int	mailmsgattent_startctpri(MAILMSGATTENT *,cchar *,int) ;
static int	mailmsgattent_startctsub(MAILMSGATTENT *,cchar *,int) ;
static int	mailmsgattent_checkatt(MAILMSGATTENT *) ;
static int	mailmsgattent_analyzer(MAILMSGATTENT *,bfile *,bfile *) ;

static int	freeit(void *) ;


/* local variables */


/* exported subroutines */


/* start a new attachment (w/ default content-type and content-encoding) */
int mailmsgattent_start(MAILMSGATTENT *op,cchar *ct,cchar *ce,
		cchar *nbuf,int nlen)
{
	int		rs = SR_OK ;
	int		si ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;

	if (nlen < 0) nlen = strlen(nbuf) ;

/* continue */

	memset(op,0,sizeof(MAILMSGATTENT)) ;
	op->cte = -1 ;
	op->clen = -1 ;
	op->clines = -1 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: cont\n") ;
#endif

	if ((si = sisub(nbuf,nlen,"=")) >= 0) {
	    const int	fl = (nlen-si-1) ;
	    cchar	*fp = (nbuf+si+1) ;
	    if ((rs = mailmsgattent_startfn(op,fp,fl)) >= 0) {
	    	rs = mailmsgattent_startct(op,nbuf,si) ;
	    }
	} else {
	    rs = mailmsgattent_startfn(op,nbuf,nlen) ;
	}

	if (rs >= 0) {
	   if ((ct != NULL) && (op->ext == NULL) && (op->type == NULL)) {
	        const int	cl = strlen(ct) ;
		rs = mailmsgattent_startct(op,ct,cl) ;
	    } /* end if */
	} /* end if (ok) */

/* content_encoding */

	if ((rs >= 0) && (ce != NULL)) {
	    cchar	*enc ;
	    if ((rs = uc_mallocstrw(ce,-1,&enc)) >= 0) {
		op->encoding = enc ;
	    }
	}

	if (rs >= 0) {
	    op->magic = MAILMSGATTENT_MAGIC ;
	} else {
	    mailmsgattent_finish(op) ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: ret rs=%d\n",rs) ;
	debugprintf("mailmsgattent_start: ret type=%s\n",op->type) ;
	debugprintf("mailmsgattent_start: ret sub=%s\n",op->subtype) ;
	debugprintf("mailmsgattent_start: ret ext=%s\n",op->ext) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgattent_start) */


int mailmsgattent_finish(MAILMSGATTENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	ep->clen = -1 ;
	ep->clines = -1 ;
	ep->cte = -1 ;

	rs1 = freeit(&ep->type) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->subtype) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->encoding) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->attfname) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->ext) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->description) ;
	if (rs >= 0) rs = rs1 ;

	if (ep->auxfname != NULL) {
	    if (ep->auxfname[0] != '\0') {
	        u_unlink(ep->auxfname) ;
	    }
	    rs1 = freeit(&ep->auxfname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (mailmsgattent_finish) */


/* find the content-type for this particular attachment */
int mailmsgattent_type(MAILMSGATTENT *ep,MIMETYPES *mtp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_type: attfname=%s\n", ep->attfname) ;
	debugprintf("mailmsgattent_type: type=%s\n",ep->type) ;
	debugprintf("mailmsgattent_type: ext=%s\n",ep->ext) ;
#endif

	if ((ep->type != NULL) && (ep->type[0] == '\0')) {
	    uc_free(ep->type) ;
	    ep->type = NULL ;
	}

	if (ep->type == NULL) {
	    cchar	*tp ;
	    char	typespec[MIMETYPES_TYPELEN + 1] ;

/* we didn't have a type so we look the hard way */

	    if (ep->subtype != NULL) {
	        uc_free(ep->subtype) ;
	        ep->subtype = NULL ;
	    }

	    if ((ep->ext != NULL) && (ep->ext[0] == '\0')) {
	        uc_free(ep->ext) ;
	        ep->ext = NULL ;
	    }

/* if there is no explicit extension, then we get it from the filename */

	    if ((ep->ext == NULL) && (ep->attfname != NULL)) {
	        if ((tp = strrchr(ep->attfname,'.')) != NULL) {
		    cchar	*cp ;
		    if ((rs = uc_mallocstrw((tp+1),-1,&cp)) >= 0) {
	                ep->ext = cp ;
		    }
	        } /* end if */
	    } /* end if (extracting filename extension) */

/* continue */

	    if (rs >= 0) {
		cchar	*vp = NULL ;
		int	vl = -1 ;
	        if (ep->ext != NULL) {
	            if ((rs = mimetypes_find(mtp,typespec,ep->ext)) >= 0) {
	                if ((tp = strchr(typespec,'/')) != NULL) {
			    cchar	*cp ;
			    vp = typespec ;
			    vl = (tp-typespec) ;
			    if ((rs = uc_mallocstrw((tp+1),-1,&cp)) >= 0) {
	                        ep->subtype = cp ;
			    }
	                } else {
	                    vp = typespec ;
		        }
	            } else if (rs == SR_NOENT) {
			rs = SR_OK ;
	                vp = "binary" ;
	            }
	        } else {
		    vp = "binary" ;
	        }
		if ((rs >= 0) && (vp != NULL)) {
		    cchar	*cp ;
		    if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
			f = TRUE ;
			ep->type = cp ;
		    }
		}
	    } /* end if (ok) */

	} /* end if (needed a type) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailmsgattent_type) */


int mailmsgattent_typeset(MAILMSGATTENT *ep,cchar *tstr,cchar *ststr)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->type != NULL) {
	    rs1 = uc_free(ep->type) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->type = NULL ;
	}

	if (ep->subtype != NULL) {
	    rs1 = uc_free(ep->subtype) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->subtype = NULL ;
	}

	if (rs >= 0) {

	    if ((tstr != NULL) && (tstr[0] != '\0')) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(tstr,-1,&cp)) >= 0) {
	            ep->type = cp ;
	            if ((ststr != NULL) && (ststr[0] != '\0')) {
	                if ((rs = uc_mallocstrw(ststr,-1,&cp)) >= 0) {
	                    ep->subtype = cp ;
	                } /* end if (memory-allocation) */
	            }
	            if (rs < 0) {
	                uc_free(ep->type) ;
	                ep->type = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (tstr) */

	    if (rs >= 0) {
	        rs = mailmsgattent_isplaintext(ep) ;
	    }

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (mailmsgattent_typeset) */


int mailmsgattent_isplaintext(MAILMSGATTENT *ep)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if ((ep->type != NULL) && (ep->subtype != NULL)) {
	    cchar	*str_text = contypevals[contypeval_text] ;
	    cchar	*str_plain = contypevals[contypeval_plain] ;
	    f = (strcmp(ep->type,str_text) == 0) ;
	    f = f && (strcmp(ep->subtype,str_plain) == 0) ;
	}
	ep->f_plaintext = f ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailmsgattent_isplaintext) */


int mailmsgattent_code(MAILMSGATTENT *ep,cchar *tmpdname)
{
	int		rs = SR_OK ;
	int		code = 0 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: ent file=%s type=%s\n",
	    ep->attfname,ep->type) ;
#endif

	if (ep->type != NULL) {
	    rs = mailmsgattent_isplaintext(ep) ;
	} else {
	    ep->f_plaintext = TRUE ;
	}

	if (! ep->f_plaintext) code = CE_BASE64 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: default_code=%d\n",code) ;
	debugprintf("mailmsgattent_code: encoding=%s\n",ep->encoding) ;
	debugprintf("mailmsgattent_code: f_plaintext=%d\n",ep->f_plaintext) ;
#endif

	if ((rs >= 0) && (ep->encoding == NULL)) {

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: need coding\n") ;
#endif

	    if (ep->f_plaintext) {

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: plaintext\n") ;
#endif

	        if ((rs = mailmsgattent_analyze(ep,tmpdname)) >= 0) {
	            code = rs ;
#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: _analyze() rs=%d\n",rs) ;
#endif
	            if ((code >= CE_7BIT) && (code < CE_OVERLAST)) {
			cchar	*enc = contentencodings[code] ;
			cchar	*cp ;
			if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	                    ep->encoding = cp ;
			}
		    }
		}

	    } else {
	        cchar	*enc = contentencodings[contentencoding_base64] ;
		cchar	*cp ;
	        code = CE_BASE64 ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    } /* end if */

	} else {
	    int		si ;
	    cchar	*enc = ep->encoding ;
	    if ((si = matstr(contentencodings,enc,-1)) >= 0) {
	        code = si ;
	    }
	}

#ifdef	COMMENT
	if ((rs < 0) || (code == 0)) {

	    if (ep->encoding != NULL) {
	        uc_free(ep->encoding) ;
	        ep->encoding = NULL ;
	    }

	    if (rs >= 0) {
	        cchar	*enc = contentencodings[contentencoding_base64] ;
		cchar	*cp ;
	        code = CE_BASE64 ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    }

	} /* end if (error) */
#else /* COMMENT */
	if (rs < 0) {
	    if (ep->encoding != NULL) {
	        uc_free(ep->encoding) ;
	        ep->encoding = NULL ;
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: ret rs=%d code=%d encoding=%s\n",
	    rs,code,ep->encoding) ;
#endif

	if (rs >= 0) {
	    ep->cte = code ;
	}

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_code) */


int mailmsgattent_setcode(MAILMSGATTENT *ep,int code)
{
	int		rs = SR_OK ;
	if ((code >= 0) && (code < CE_OVERLAST)) {
	    cchar	*enc ;
	    ep->cte = code ;
	    if (ep->encoding == NULL) {
		cchar	*cp ;
	        enc = contentencodings[code] ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    }
	} else {
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (mailmsgattent_setcode) */


/* analyze (as best as we can) an attachment */
int mailmsgattent_analyze(MAILMSGATTENT *ep,cchar *tmpdname)
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		code = 0 ;
	const char	*atf = ep->attfname ;

	if ((atf == NULL) || (atf[0] == '\0') || (atf[0] == '-'))
	    atf = BFILE_STDIN ;

	if ((rs = bopen(ifp,atf,"r",0666)) >= 0) {
	    bfile_stat	sb ;
	    if ((rs = bcontrol(ifp,BC_STAT,&sb)) >= 0) {
		bfile	auxfile, *afp = &auxfile ;
		int	f_needaux = TRUE ;
		char	auxfname[MAXPATHLEN+1] ;

		auxfname[0] = '\0' ;
	        if (S_ISREG(sb.st_mode)) {
#if	CF_DEBUGS
	            debugprintf("mailmsgattent_analyze: needaux=%d\n",
			f_needaux) ;
#endif
	            ep->clen = (int) sb.st_size ;
	            f_needaux = FALSE ;
	        }

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_analyze: needaux=%d\n",f_needaux) ;
#endif

	        if (f_needaux) {
	            const char	*tmpcname = MAILMSGATTENT_TMPCNAME ;
	            char	tmpfname[MAXPATHLEN + 1] ;

	            if (tmpdname == NULL)
	                rs = SR_FAULT ;

	            if (rs >= 0) {
	                if (tmpdname[0] != '\0') {
	                    rs = mkpath2(tmpfname,tmpdname,tmpcname) ;
	                } else {
	                    rs = mkpath1(tmpfname,tmpcname) ;
			}
	            }

	            if (rs >= 0) {
	                if ((rs = mktmpfile(auxfname,0660,tmpfname)) >= 0) {
			    cchar	*afn = auxfname ;
			    cchar	*cp ;
			    if ((rs = uc_mallocstrw(afn,-1,&cp)) >= 0) {
				ep->auxfname = cp ;
	                        rs = bopen(afp,ep->auxfname,"wct",0666) ;
	                        if (rs < 0) {
	                            uc_free(ep->auxfname) ;
	                            ep->auxfname = NULL ;
	                        }
			    } /* end if (m-a) */
	                    if (rs < 0) {
	                        uc_unlink(auxfname) ;
	                        auxfname[0] = '\0' ;
	                    }
	                } /* end if (mktmpfile) */
	            } /* end if (ok) */

	        } /* end if (needed an auxillary file) */

/* finally! perform the analysis */

	        if (rs >= 0) {

	            if (! f_needaux) afp = NULL ;
	            rs = mailmsgattent_analyzer(ep,afp,ifp) ;
		    code = rs ;

#if	CF_DEBUGS
	            debugprintf("mailmsgattent_analyze: _analyzer() rs=%d\n",
			rs) ;
#endif
	            if (rs < 0) {
	                if (ep->auxfname != NULL) {
	                    uc_free(ep->auxfname) ;
	                    ep->auxfname = NULL ;
	                }
	                if (auxfname[0] != '\0') {
	                    u_unlink(auxfname) ;
			}
	            }
	        }

	        if (f_needaux) {
	            if (afp != NULL) bclose(afp) ;
	        } else {
	            bseek(ifp,0L,SEEK_SET) ;
		}

	    } /* end if (stat) */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyze: ret rs=%d ce=%u\n",rs,code) ;
#endif

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_analyze) */


/* private subroutines */


static int mailmsgattent_startfn(MAILMSGATTENT *op,cchar *sp,int sl)
{
	int		rs ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    cchar	*fn ;
	    if ((rs = uc_mallocstrw(cp,cl,&fn)) >= 0) {
	        op->attfname = fn ;
		rs = mailmsgattent_checkatt(op) ;
		if (rs < 0) {
		    uc_free(op->attfname) ;
		    op->attfname = NULL ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (mailmsgattent_startfn) */


static int mailmsgattent_startct(MAILMSGATTENT *op,cchar *sp,int sl)
{
	int		rs ;
	int		si ;
	if ((si = sisub(sp,sl,"/")) >= 0) {
	    if ((rs = mailmsgattent_startctpri(op,sp,si)) >= 0) {
	        const int	cl = (sl-(si+1)) ;
		cchar		*cp = (sp+si+1) ;
	        rs = mailmsgattent_startctsub(op,cp,cl) ;
	    }
	} else {
	    int		el ;
	    cchar	*ep ;
	    if ((el = sfshrink(sp,sl,&ep)) > 0) {
	        int	oi ;
	        if ((oi = matcasestr(contypevals,ep,el)) >= 0) {
	            rs = mailmsgattent_startctpri(op,ep,el) ;
	        } else {
		    cchar	*ext ;
	            if ((rs = uc_mallocstrw(ep,el,&ext)) >= 0) {
	                op->ext = ext ;
	            }
	        }
	    }
	}

	return rs ;
}
/* end subroutine (mailmsgattent_startct) */


static int mailmsgattent_startctpri(MAILMSGATTENT *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    cchar	*fn ;
	    if ((rs = uc_mallocstrw(cp,cl,&fn)) >= 0) {
	        op->type = fn ;
	    }
	}
	return rs ;
}
/* end subroutine (mailmsgattent_startctpri) */


static int mailmsgattent_startctsub(MAILMSGATTENT *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    cchar	*fn ;
	    if ((rs = uc_mallocstrw(cp,cl,&fn)) >= 0) {
	        op->subtype = fn ;
	    }
	}
	return rs ;
}
/* end subroutine (mailmsgattent_startctsub) */


static int mailmsgattent_checkatt(MAILMSGATTENT *op)
{
	int		rs = SR_OK ;
	cchar		*fn = op->attfname ;
	if (fn != NULL) {
	    if (strcmp(fn,"-") != 0) {
	        rs = perm(fn,-1,-1,NULL,R_OK) ;
	    }
	}
	return rs ;
}
/* end subroutine (mailmsgattent_checkatt) */


static int mailmsgattent_analyzer(MAILMSGATTENT *ep,bfile *afp,bfile *ifp)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		lines = 0 ;
	int		len ;
	int		clen = 0 ;
	int		code = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyzer: ent\n") ;
#endif

	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    if (code < CE_BINARY) {
		int	i ;
		int	ch ;
	        for (i = 0 ; i < len ; i += 1) {
	            ch = MKCHAR(lbuf[i]) ;

	            if (ismmclass_binary(ch)) {
	                code = CE_BINARY ;
	            } else if ((code < CE_8BIT) && ismmclass_8bit(ch)) {
	                code = CE_8BIT ;
	            }

#if	CF_DEBUGS
		    debugprintf("mailmsgattent_analyzer: ch=%02x code=%d\n",
			ch,code) ;
#endif

		    if (code >= CE_BINARY) break ;
	        } /* end for (class characterization) */
	    } /* end if (continue) */

	    if (afp != NULL) {
	        rs = bwrite(afp,lbuf,len) ;
	    } else {
	        if (code >= CE_BINARY) break ;
	    }

	    clen += len ;
	    if (lbuf[len - 1] == '\n') lines += 1 ;

	    if (rs < 0) break ;
	} /* end while */

	if (rs >= 0) {
	    if (ep->clen < 0) ep->clen = clen ;
	    ep->clines = (code < CE_BINARY) ? lines : -1 ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyzer: ret rs=%d code=%u\n",rs,code) ;
#endif

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_analyzer) */


static int freeit(void *p)
{
	int		rs = SR_OK ;
	void		**pp = (void **) p ;
	if (*pp != NULL) {
	    rs = uc_free(*pp) ;
	    *pp = NULL ;
	}
	return rs ;
}
/* end subroutine (freeit) */


