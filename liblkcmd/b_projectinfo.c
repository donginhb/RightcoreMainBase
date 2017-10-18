/* b_projectinfo */

/* SHELL built-in: return various group information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCSETENT	1		/* |locinfo_setentry()| */


/* revision history:

	= 2004-03-01, David A�D� Morano
	This subroutine was originally written.  It was inspired by many
	programs that performs various subset functions of this program.  This
	can be either a KSH builtin or a stand-alone program.

	= 2010-02-03, David A�D� Morano
	This subroutine was changed to acess information from the system-group
	database.

*/

/* Copyright � 2004,2010 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

	We query and present information from the system PROJECT database.

	Synopsis:

	$ projectinfo [[<groupname>|-] <qkey(s)>] [-af <afile>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<project.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<estrings.h>
#include	<ctdec.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<sbuf.h>
#include	<getax.h>
#include	<sysusers.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_projectinfo.h"
#include	"defs.h"


/* local defines */

#ifndef	VBUFLEN
#define	VBUFLEN		MAXNAMELEN
#endif

#ifndef	VARHZ
#define	VARHZ		"HZ"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARNAME
#define	VARNAME		"NAME"
#endif

#ifndef	VARFULLNAME
#define	VARFULLNAME	"FULLNAME"
#endif

#ifndef	VARMAILNAME
#define	VARMAILNAME	"MAILNAME"
#endif

#ifndef	VARPRINTER
#define	VARPRINTER	"PRINTER"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpylc(char *,int,cchar *) ;
extern int	sncpyuc(char *,int,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecpstr_loadgrusers(VECPSTR *,gid_t) ;
extern int	vecpstr_loadpjusers(VECPSTR *,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isInvalid(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		dymmy:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	struct project	pj ;
	PROGINFO	*pip ;
	int		pjlen ;
	cchar		*projname ;
	char		*pjbuf ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,void *,cchar *) ;
static int	proclist(PROGINFO *,void *) ;
static int	proclistfind(PROGINFO *,vecpstr *) ;
static int	proclistusers(PROGINFO *,vecpstr *) ;
static int	proclistgroups(PROGINFO *,vecpstr *) ;
static int	proclistgroup(PROGINFO *,vecpstr *,char *,int,cchar *) ;
static int	procqueries(PROGINFO *,void *,cchar *,int) ;
static int	procquery(PROGINFO *,void *,cchar *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setproject(LOCINFO *,cchar *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */

static int	mklist(char *,int,char **) ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

/* define the query keywords */
static cchar *qopts[] = {
	"pjid",
	"name",
	"projectname",
	"comment",
	"description",
	"users",
	"groups",
	NULL
} ;

enum qopts {
	qopt_pjid,
	qopt_name,
	qopt_projectname,
	qopt_comment,
	qopt_description,
	qopt_users,
	qopt_groups,
	qopt_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int b_projectinfo(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_projectinfo) */


int p_projectinfo(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_projectinfo) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos, ai_continue ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*projname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("projectinfo: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        pip->final.all = TRUE ;
	                        pip->have.all = TRUE ;
	                        pip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'l':
	                        pip->final.list = TRUE ;
	                        pip->have.list = TRUE ;
	                        pip->f.list = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.list = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("projectinfo: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* get the first positional argument as the username to key on */

	if (pip->f.all) pip->f.list = TRUE ;

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	ai_continue = 1 ;
	for (ai = ai_continue ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        projname = argv[ai] ;
	        ai_continue = (ai + 1) ;
	        break ;
	    }
	} /* end for */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: proj=%s\n",pn,projname) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;
	ainfo.ai_continue = ai_continue ;

	if (rs >= 0) {
	    if ((rs = locinfo_setproject(lip,projname)) >= 0) {
	        cchar	*ofn = ofname ;
	        cchar	*afn = afname ;
	        rs = process(pip,&ainfo,&pargs,ofn,afn) ;
	        if (rs < 0) {
	            if (rs == SR_NOTFOUND) ex = EX_NOTFOUND ;
	        }
	    } /* end if (locinfo_setproject) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_NOANODE:
	        ex = EX_OLDER ;
	        break ;
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_SEARCH:
	        ex = EX_NOUSER ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if (rs >= 0) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("projectinfo: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [{<projectname>|<pjid>|-}] [{<query(s)>|-a}]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:    possible query keywords are:\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	for (i = 0 ; (rs >= 0) && (qopts[i] != NULL) ; i += 1) {

	    if ((i % USAGECOLS) == 0) {
	        rs = shio_printf(pip->efp,"%s: \t",pip->progname) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = shio_printf(pip->efp,"%-16s",qopts[i]) ;
	        wlen += rs ;
	        if ((rs >= 0) && ((i % USAGECOLS) == 3)) {
	            rs = shio_printf(pip->efp,"\n") ;
	            wlen += rs ;
	        }
	    }

	} /* end for */

	if ((i % USAGECOLS) != 0) {
	    if (rs >= 0) rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
	    cchar	*pn = pip->progname ;

/* handle list mode */

	    if (pip->f.list) {
	        if (pip->debuglevel > 0) {
	    	    cchar	*ms = (pip->f.all) ? "all" : "list" ;
	            shio_printf(pip->efp,"%s: mode=%s\n",pn,ms) ;
	        }
	        rs = proclist(pip,ofp) ;
	        wlen += rs ;
	    } else {
	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,"%s: mode=query\n",pn) ;
	        }
	        rs = procargs(pip,aip,bop,ofp,afn) ;
	        wlen += rs ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		wlen = 0 ;
	cchar		*cp ;

	if (rs >= 0) {
	    cchar	**argv = aip->argv ;
	    int		ai ;
	    int		f ;
	    for (ai = aip->ai_continue ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procquery(pip,ofp,cp,-1) ;
	                wlen += rs ;
	            }
	        }

	        if (rs >= 0) rs = lib_sigterm() ;
	        if (rs >= 0) rs = lib_sigintr() ;
	        if (rs < 0) break ;
	    } /* end for (handling positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = procqueries(pip,ofp,cp,cl) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "projectname" ;
	    pan += 1 ;
	    rs = procquery(pip,ofp,cp,-1) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procqueries(PROGINFO *pip,void *ofp,cchar *lbuf,int len)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procquery(pip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procqueries) */


/* process a query specification */
static int procquery(PROGINFO *pip,void *ofp,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 = SR_NOENT ;
	int		ci ;
	int		v ;
	int		vl = -1 ;
	int		wlen = 0 ;
	cchar		*vp = NULL ;
	char		vbuf[VBUFLEN + 1] ;

	vbuf[0] = '\0' ;
	ci = matostr(qopts,1,np,nl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procquery: query=%t i=%d\n",
	        np,nl,ci) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: query=%t(%d)\n",
	        pip->progname,np,nl,ci) ;
	}

	switch (ci) {
	case qopt_pjid:
	    v = lip->pj.pj_projid ;
	    if ((rs1 = ctdeci(vbuf,vlen,v)) >= 0) {
	        vp = vbuf ;
	        vl = rs1 ;
	    }
	    break ;
	case qopt_name:
	case qopt_projectname:
	    vp = lip->pj.pj_name ;
	    break ;
	case qopt_comment:
	case qopt_description:
	    vp = lip->pj.pj_comment ;
	    break ;
	case qopt_users:
	    if ((rs = mklist(vbuf,vlen,lip->pj.pj_users)) >= 0) {
	        vp = vbuf ;
	        vl = rs ;
	    }
	    break ;
	case qopt_groups:
	    if ((rs = mklist(vbuf,vlen,lip->pj.pj_groups)) >= 0) {
	        vp = vbuf ;
	        vl = rs ;
	    }
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

/* print out */

	if ((rs >= 0) && (pip->verboselevel > 0)) {

	    if (vp == NULL) {
	        vp = "*" ;
	        vl = 1 ;
	    }

	    if (vl < 0) vl = strlen(vp) ;

	    rs = shio_print(ofp,vp,vl) ;
	    wlen += rs ;

	} /* end if (printing out) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */


static int proclist(PROGINFO *pip,void *ofp)
{
	vecpstr		pjl ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = vecpstr_start(&pjl,8,256,0)) >= 0) {

	    if ((rs = proclistfind(pip,&pjl)) >= 0) {
	        int	i ;
	        cchar	*up ;
	        for (i = 0 ; vecpstr_get(&pjl,i,&up) >= 0 ; i += 1) {
	            rs = shio_print(ofp,up,-1) ;
	            wlen += rs ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (proclistfind) */

	    rs1 = vecpstr_finish(&pjl) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecpstr) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (proclist) */


static int proclistfind(PROGINFO *pip,vecpstr *plp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;
	cchar		*pjn ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("projectinfo/proclistfind: ent\n") ;
#endif

	pjn = lip->projname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("projectinfo/proclistfind: pjn=%s\n",pjn) ;
#endif

	if ((rs = vecpstr_loadpjusers(plp,pjn)) >= 0) {
	    c += rs ;
	    if (pip->f.all) {
	        if ((rs = proclistusers(pip,plp)) >= 0) {
	            c += rs ;
	            if ((rs = proclistgroups(pip,plp)) >= 0) {
	                c += rs ;
	            }
	        }
	    } /* end if (all) */
	} /* end if (vecpstr_loadpjusers) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("projectinfo/proclistfind: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclistfind) */


static int proclistusers(PROGINFO *pip,vecpstr *plp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		**users ;
	users = (cchar **) lip->pj.pj_users ;
	if (users != NULL) {
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (users[i] != NULL) ; i += 1) {
	        rs = vecpstr_adduniq(plp,users[i],-1) ;
	        if (rs < INT_MAX) c += 1 ;
	    } /* end for */
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclistusers) */


static int proclistgroups(PROGINFO *pip,vecpstr *plp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		**groups ;
	groups = (cchar **) lip->pj.pj_groups ;
	if (groups != NULL) {
	    const int	grlen = getbufsize(getbufsize_gr) ;
	    char	*grbuf ;
	    if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (groups[i] != NULL) ; i += 1) {
	            rs = proclistgroup(pip,plp,grbuf,grlen,groups[i]) ;
	            c += rs ;
	        } /* end for */
	        uc_free(grbuf) ;
	    } /* end if (m-a) */
	} /* end if (non-NULL) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclistgroups) */


static int proclistgroup(PROGINFO *pip,vecpstr *plp,
		char *grbuf,int grlen,cchar *gn)
{
	struct group	gr ;
	int		rs ;
	int		c = 0 ;
	if ((rs = getgr_name(&gr,grbuf,grlen,gn)) >= 0) {
	    const gid_t	gid = gr.gr_gid ;
	    if ((rs = vecpstr_loadgrusers(plp,gid)) >= 0) {
	        c += rs ;
	        if (gr.gr_mem != NULL) {
	            int		i ;
	            cchar	**mems = (cchar **) gr.gr_mem ;
	            for (i = 0 ; (rs >= 0) && (mems[i] != NULL) ; i += 1) {
	                rs = vecpstr_adduniq(plp,mems[i],-1) ;
	                if (rs < INT_MAX) c += 1 ;
	            } /* end for */
	        } /* end if (had members) */
	    } /* end if (vecpstr_loadgrusers) */
	} /* end if (getgr_name) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclistgroup) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	const int	pjlen = getbufsize(getbufsize_pj) ;
	int		rs ;
	char		*bp ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	if ((rs = uc_malloc((pjlen+1),&bp)) >= 0) {
	    lip->pjbuf = bp ;
	    lip->pjlen = pjlen ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->pjbuf != NULL) {
	    rs1 = uc_free(lip->pjbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->pjbuf = NULL ;
	    lip->pjlen = 0 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_setproject(LOCINFO *lip,cchar *pjn)
{
	PROGINFO	*pip = lip->pip ;
	projid_t	pjid ;
	int		rs = SR_OK ;
	int		f_self = FALSE ;

	if (pip == NULL) return SR_FAULT ;

	if ((pjn == NULL) || (pjn[0] == '\0') || (strcmp(pjn,"-") == 0)) {
	    f_self = TRUE ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_projectinfo/locinfo_setproject: f_self=%u\n",
		f_self) ;
#endif

	if (f_self) {
	    struct project	*pjp = &lip->pj ;
	    char		*pjbuf = lip->pjbuf ;
	    const int		pjlen = lip->pjlen ;
	    pjid = getprojid() ;
	    if ((rs = getpj_pjid(pjp,pjbuf,pjlen,pjid)) >= 0) {
		pjn = pjp->pj_name ;
	    }
	} else {
	    struct project	*pjp = &lip->pj ;
	    char		*pjbuf = lip->pjbuf ;
	    const int		pjlen = lip->pjlen ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("projectinfo: pjn=%s\n",pjn) ;
#endif

	    if (hasalldig(pjn,-1)) {
	        int	v ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_projectinfo/locinfo_setproject: alldig\n") ;
#endif

	        if ((rs = cfdeci(pjn,-1,&v)) >= 0) {
	            pjid = v ;
	            if ((rs = getpj_pjid(pjp,pjbuf,pjlen,pjid)) >= 0) {
			pjn = pjp->pj_name ;
		    }
	        }

	        if ((rs == SR_NOTFOUND) || isInvalid(rs)) {
	            rs = getpj_name(pjp,pjbuf,pjlen,pjn) ;
	        }

	    } else {
	        rs = getpj_name(pjp,pjbuf,pjlen,pjn) ;
	    }

	} /* end if */

	if (rs >= 0) {
	    cchar	**vpp = &lip->projname ;
	    rs = locinfo_setentry(lip,vpp,pjn,-1) ;
	}

	return rs ;
}
/* end subroutine (locinfo_setproject) */


static int mklist(char *rbuf,int rlen,char **names)
{
	SBUF		b ;
	int		rs ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int		len ;
	    int		i ;

	    for (i = 0 ; (rs >= 0) && (names[i] != NULL) ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,' ') ;
	        if (rs >= 0) rs = sbuf_strw(&b,names[i],-1) ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mklist) */


