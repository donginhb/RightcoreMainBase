/* main */

/* front-end subroutine for the FILEOP program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FOLLOWFILES	0		/* follow symbolic links of files */
#define	CF_FTCASE	1		/* try a C-lang 'switch' */
#define	CF_DIRS		1		/* dirs */
#define	CF_NEWPRUNE	1		/* try new-prune */


/* revision history:

	= 1998-02-01, David A�D� Morano

	The program was written from scratch to do what the previous program by
	the same name did.


*/

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<baops.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<fsdirtree.h>
#include	<sigblock.h>
#include	<bwops.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	DMODE		0775

#define	DIRID		struct dirid
#define	FILEID		struct fileid

#define	LINKINFO	struct linkinfo

#define	FILEINFO	struct fileinfo
#define	FILEINFO_FL	struct fileinfo_flags


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	removes(cchar *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mkuserpath(char *,cchar *,cchar *,int) ;
extern int	fileobject(cchar *) ;
extern int	strwcmp(cchar *,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;


/* local structures */

struct fileinfo_flags {
	uint		dangle:1 ;
} ;

struct fileinfo {
	FILEINFO_FL	f ;
	uint		fts ;
} ;

struct dirid {
	uino_t		ino ;
	dev_t		dev ;
} ;

struct fileid {
	uino_t		ino ;
	dev_t		dev ;
} ;

struct linkinfo {
	uino_t		ino ;
	cchar		*fname ;
} ;


/* forward references */

static uint	linkhash(const void *,int) ;
static uint	diridhash(const void *,int) ;
static uint	fileidhash(const void *,int) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *) ;
static int	procfts(PROGINFO *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procname(PROGINFO *,cchar *) ;
static int	procdir(PROGINFO *,cchar *,FSDIRTREE_STAT *) ;
static int	procother(PROGINFO *,cchar *,FSDIRTREE_STAT *) ;
static int	procprune(PROGINFO *,cchar *) ;
static int	procprintfts(PROGINFO *,cchar *) ;
static int	procprintsufs(PROGINFO *,cchar *) ;

static int	procsuf_begin(PROGINFO *) ;
static int	procsuf_load(PROGINFO *,int,cchar *,int) ;
static int	procsuf_have(PROGINFO *,cchar *,int) ;
static int	procsuf_end(PROGINFO *) ;

static int	procrm_begin(PROGINFO *) ;
static int	procrm_add(PROGINFO *,cchar *,int) ;
static int	procrm_end(PROGINFO *) ;

static int	procdir_begin(PROGINFO *) ;
static int	procdir_have(PROGINFO *,dev_t,uino_t,cchar *,int) ;
static int	procdir_addid(PROGINFO *,dev_t,uino_t) ;
static int	procdir_haveprefix(PROGINFO *,cchar *,int) ;
static int	procdir_addprefix(PROGINFO *,cchar *,int) ;
static int	procdir_end(PROGINFO *) ;

static int	procuniq_begin(PROGINFO *) ;
static int	procuniq_end(PROGINFO *) ;
static int	procuniq_have(PROGINFO *,dev_t,uino_t) ;
static int	procuniq_addid(PROGINFO *,dev_t,uino_t) ;

static int	procprune_begin(PROGINFO *,cchar *) ;
static int	procprune_end(PROGINFO *) ;
static int	procprune_loadfile(PROGINFO *,cchar *) ;
static int	procprune_size(PROGINFO *,int *) ;

static int	proclink_begin(PROGINFO *) ;
static int	proclink_add(PROGINFO *,uino_t,cchar *) ;
static int	proclink_have(PROGINFO *,uino_t,LINKINFO **) ;
static int	proclink_end(PROGINFO *) ;

static int	procsize(PROGINFO *,cchar *, 
			FSDIRTREE_STAT *,FILEINFO *) ;
static int	proclink(PROGINFO *,cchar *, 
			FSDIRTREE_STAT *,FILEINFO *) ;
static int	procsync(PROGINFO *,cchar *, 
			FSDIRTREE_STAT *,FILEINFO *) ;
static int	procrm(PROGINFO *,cchar *, 
			FSDIRTREE_STAT *,FILEINFO *) ;

static int	procsynclink(PROGINFO *,cchar *,
			FSDIRTREESTAT *,LINKINFO *) ;
static int	procsyncer(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;

static int	procsyncer_reg(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;
static int	procsyncer_dir(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;
static int	procsyncer_lnk(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;
static int	procsyncer_fifo(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;
static int	procsyncer_sock(PROGINFO *,cchar *,
			FSDIRTREESTAT *) ;

static int	linkinfo_start(LINKINFO *,uino_t,cchar *) ;
static int	linkinfo_finish(LINKINFO *) ;

static int	dirid_start(DIRID *,dev_t,uino_t) ;
static int	dirid_finish(DIRID *) ;

static int	fileid_start(FILEID *,dev_t,uino_t) ;
static int	fileid_finish(FILEID *) ;

static int	mkpdirs(cchar *,mode_t) ;
static int	istermrs(int) ;

static int	linkcmp(struct linkinfo *,struct linkinfo *,int) ;
static int	diridcmp(struct dirid *,struct dirid *,int) ;
static int	fileidcmp(struct fileid *,struct fileid *,int) ;

static int	vcmprstr(cchar **e1pp,cchar **e2pp) ;

static int	isAccess(int) ;


/* external variables */


/* local structures */

enum fts {
	ft_r,
	ft_d,
	ft_b,
	ft_c,
	ft_p,
	ft_n,
	ft_l,
	ft_s,
	ft_D,
	ft_e,
	ft_overlast
} ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"pf",
	"yf",
	"sa",
	"sr",
	"yi",
	"option",
	"set",
	"follow",
	"cores",
	"iacc",
	"ignacc",
	"im",
	"nice",
	"ff",
	"readable",
	"older",
	"accessed",
	"prune",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_pf,
	argopt_yf,
	argopt_sa,
	argopt_sr,
	argopt_yi,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_cores,
	argopt_iacc,
	argopt_ignacc,
	argopt_im,
	argopt_nice,
	argopt_ff,
	argopt_readable,
	argopt_older,
	argopt_accessed,
	argopt_prune,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
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

static cchar	*progmodes[] = {
	"filesize",
	"filefind",
	"filelinker",
	"filesyncer",
	"filerm",
	NULL
} ;

enum progmodes {
	progmode_filesize,
	progmode_filefind,
	progmode_filelinker,
	progmode_filesyncer,
	progmode_filerm,
	progmode_overlast
} ;

static cchar	*progopts[] = {
	"uniq",
	"name",
	"noprog",
	"nosock",
	"nopipe",
	"nofifo",
	"nodev",
	"noname",
	"nolink",
	"noextra",
	"nodotdir",
	"cores",
	"s",
	"sa",
	"sr",
	"nosuf",
	"follow",
	"younger",
	"yi",
	"iacc",
	"ignacc",
	"im",
	"quiet",
	"nice",
	"ff",
	"readable",
	"older",
	"accessed",
	"prune",
	NULL
} ;

enum progopts {
	progopt_uniq,
	progopt_name,
	progopt_noprog,
	progopt_nosock,
	progopt_nopipe,
	progopt_nofifo,
	progopt_nodev,
	progopt_noname,
	progopt_nolink,
	progopt_noextra,
	progopt_nodotdir,
	progopt_cores,
	progopt_s,
	progopt_sa,
	progopt_sr,
	progopt_nosuf,
	progopt_follow,
	progopt_younger,
	progopt_yi,
	progopt_iacc,
	progopt_ignacc,
	progopt_im,
	progopt_quiet,
	progopt_nice,
	progopt_ff,
	progopt_readable,
	progopt_older,
	progopt_accessed,
	progopt_prune,
	progopt_overlast
} ;

static cchar	*ftstrs[] = {
	"file",
	"directory",
	"block",
	"character",
	"pipe",
	"fifo",
	"name",
	"socket",
	"link",
	"door",
	"exists",
	"regular",
	NULL
} ;

enum ftstrs {
	ftstr_file,
	ftstr_directory,
	ftstr_block,
	ftstr_character,
	ftstr_pipe,
	ftstr_fifo,
	ftstr_name,
	ftstr_socket,
	ftstr_link,
	ftstr_door,
	ftstr_exists,
	ftstr_regular,
	ftstr_overlast
} ;

#ifdef	COMMENT

static cchar	*whiches[] = {
	"uniq",
	"name",
	"noprog",
	"nosock",
	"nopipe",
	"nodev",
	"nolink",
	NULL
} ;

enum whiches {
	which_uniq,
	which_name,
	which_noprog,
	which_nosock,
	which_nopipe,
	which_nodev,
	which_nolink,
	which_overlast
} ;

#endif /* COMMENT */

static cchar	*po_fts = PO_FTS ;
static cchar	*po_sufreq = PO_SUFREQ ;
static cchar	*po_sufacc = PO_SUFACC ;
static cchar	*po_sufrej = PO_SUFREJ ;
static cchar	*po_prune = PO_PRUNE ;

static cchar	*sufs[] = {
	PO_SUFREQ,
	PO_SUFACC,
	PO_SUFREJ,
	NULL
} ;

enum sufs {
	suf_req,
	suf_acc,
	suf_rej,
	suf_overlast
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;

static const int	accrs[] = {
	SR_ACCESS,
	SR_PERM,
	0
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*pmspec = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*yfname = NULL ;
	cchar		*pfname = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->namelen = MAXNAMELEN ;
	pip->verboselevel = 1 ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;
	pip->progmode = -1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&pip->akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&pip->aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            KEYOPT	*kop = &pip->akopts ;
	                            rs = keyopt_loads(kop,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
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

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_set:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            PARAMOPT	*app = &pip->aparams ;
	                            rs = paramopt_loadu(app,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pmspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* search name */
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

/* argument files */
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

/* error file */
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

/* output file */
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

/* prune-file */
	                case argopt_pf:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* younger-file */
	                case argopt_yf:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            yfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->final.follow = TRUE ;
	                    pip->have.follow = TRUE ;
	                    pip->f.follow = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.follow = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_cores:
	                    pip->final.cores = TRUE ;
	                    pip->f.cores = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.cores = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* ignore inaccessible files */
	                case argopt_iacc:
	                case argopt_ignacc:
	                    pip->final.iacc = TRUE ;
	                    pip->f.iacc = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.iacc = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_im:
	                    pip->final.im = TRUE ;
	                    pip->f.im = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.im = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* nice value */
	                case argopt_nice:
	                    cp = NULL ;
	                    cl = -1 ;
	                    pip->final.nice = TRUE ;
	                    pip->have.nice = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cp = argp ;
	                                cl = argl ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    if ((rs >= 0) && (cp != NULL)) {
	                        rs = optvalue(cp,cl) ;
	                        pip->nice = rs ;
	                    }
	                    break ;

/* first-follow */
	                case argopt_ff:
	                    pip->final.ff = TRUE ;
	                    pip->have.ff = TRUE ;
	                    pip->f.ff = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.ff = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* readable */
	                case argopt_readable:
	                    pip->final.readable = TRUE ;
	                    pip->have.readable = TRUE ;
	                    pip->f.readable = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.readable = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* older */
	                case argopt_older:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.older = TRUE ;
	                            pip->have.older = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->older = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* accessed */
	                case argopt_accessed:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.accessed = TRUE ;
	                            pip->have.accessed = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->accessed = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* suffix-accept */
	                case argopt_sa:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.sufacc = TRUE ;
	                            rs = procsuf_load(pip,suf_acc,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* suffix-reject */
	                case argopt_sr:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->final.sufrej = TRUE ;
	                            rs = procsuf_load(pip,suf_rej,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* younger-interval */
	                case argopt_yi:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->have.younger = TRUE ;
	                            pip->final.younger = TRUE ;
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->younger = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* prune components */
	                case argopt_prune:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            PARAMOPT	*app = &pip->aparams ;
	                            cchar	*po = po_prune ;
	                            rs = paramopt_loads(app,po,argp,argl) ;
				    pip->f.prune |= (rs > 0) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

/* quiet */
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

	                    case 'V':
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;
	                        break ;

/* continue on error */
	                    case 'c':
	                        pip->final.nostop = TRUE ;
	                        pip->have.nostop = TRUE ;
	                        pip->f.nostop = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.nostop = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* target directory */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->final.tardname = TRUE ;
	                                pip->have.tardname = TRUE ;
	                                pip->tardname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* follow symbolic links */
	                    case 'f':
	                        pip->final.follow = TRUE ;
	                        pip->have.follow = TRUE ;
	                        pip->f.follow = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.follow = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* file name length restriction */
	                    case 'l':
	                        cp = NULL ;
	                        cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    cp = argp ;
	                                    cl = argl ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        if ((rs >= 0) && (cp != NULL)) {
	                            rs = optvalue(cp,cl) ;
	                            pip->namelen = rs ;
	                        }
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                KEYOPT	*kop = &pip->akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->final.quiet = TRUE ;
	                        pip->have.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.quiet = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* remove before over-writing */
	                    case 'r':
	                        pip->final.rmfile = TRUE ;
	                        pip->have.rmfile = TRUE ;
	                        pip->f.rmfile = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.rmfile = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                        cp = NULL ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    cp = argp ;
	                                    cl = argl ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        if ((rs >= 0) && (cp != NULL)) {
	                            pip->final.sufreq = TRUE ;
	                            rs = procsuf_load(pip,suf_req,cp,cl) ;
	                        }
	                        break ;

/* file types */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*app = &pip->aparams ;
	                                cchar		*po = po_fts ;
	                                rs = paramopt_loads(app,po,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* unique */
	                    case 'u':
	                        pip->final.f_uniq = TRUE ;
	                        pip->have.f_uniq = TRUE ;
	                        pip->f.f_uniq = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.f_uniq = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* verbose output */
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

	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->final.younger = TRUE ;
	                                pip->have.younger = TRUE ;
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->younger = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* allow zero number of arguments */
	                    case 'z':
	                        pip->final.zargs = TRUE ;
	                        pip->final.zargs = TRUE ;
	                        pip->f.zargs = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.zargs = (rs > 0) ;
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

	        } /* end if (digits or progopts) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

/* get our program mode */

	if (pmspec == NULL) pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filesize ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: progmode=%s(%u)\n",
	        pip->progname,progmodes[pip->progmode],pip->progmode) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	pip->daytime = time(NULL) ;
	pip->euid = geteuid() ;
	pip->uid = getuid() ;

/* younger interval? */

	if ((rs >= 0) && (pip->younger == 0)) {
	    if (argval != NULL) {
	        pip->have.younger = TRUE ;
	        rs = cfdecti(argval,-1,&v) ;
	        pip->younger = v ;
	    }
	}

	if ((rs >= 0) && (pip->younger == 0)) {
	    if ((yfname != NULL) && (yfname[0] != '\0')) {
	        struct ustat	sb ;
	        if ((rs1 = uc_stat(yfname,&sb)) >= 0) {
	            pip->have.younger = TRUE ;
	            pip->younger = (pip->daytime - sb.st_mtime) ;
	        }
	    }
	}

/* get more program options */

	if (rs >= 0)
	    rs = procopts(pip) ;

	if (rs >= 0)
	    rs = procfts(pip) ;

	if ((rs >= 0) && pip->f.cores) {
	    pip->fts |= (1 << ft_r) ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    rs = procprintfts(pip,po_fts) ;
	}

	if (rs >= 0) {
	    cchar	*vp = getourenv(envv,VARPRUNE) ;
	    if (vp != NULL) {
	        PARAMOPT	*pop = &pip->aparams ;
	        cchar		*po = po_prune ;
	        rs = paramopt_loads(pop,po,vp,-1) ;
	        pip->f.prune |= (rs > 0) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (pip->f.f_noextra) {
	    pip->f.f_nodev = TRUE ;
	    pip->f.f_nopipe = TRUE ;
	    pip->f.f_nosock = TRUE ;
	    pip->f.f_noname = TRUE ;
	    pip->f.f_nodoor = TRUE ;
	}

/* create the 'fnos' value */

	if (pip->f.f_nodev) {
	    bwset(pip->fnos,ft_c) ;
	    bwset(pip->fnos,ft_b) ;
	}
	if (pip->f.f_noname) bwset(pip->fnos,ft_n) ;
	if (pip->f.f_nopipe) bwset(pip->fnos,ft_p) ;
	if (pip->f.f_nolink) bwset(pip->fnos,ft_l) ;
	if (pip->f.f_nosock) bwset(pip->fnos,ft_s) ;
	if (pip->f.f_nodoor) bwset(pip->fnos,ft_d) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: f_follow=%u\n",pip->f.follow) ;
	    debugprintf("main: f_continue=%u\n",pip->f.nostop) ;
	    debugprintf("main: f_iacc=%u\n",pip->f.iacc) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: follow=%u\n",pn,pip->f.follow) ;
	    bprintf(pip->efp,"%s: continue=%u\n",pn,pip->f.nostop) ;
	    bprintf(pip->efp,"%s: iacc=%u\n",pn,pip->f.iacc) ;
	}

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs >= 0) {
	    int	f = FALSE ;

	    f = f || (pip->progmode == progmode_filelinker) ;
	    f = f || (pip->progmode == progmode_filesyncer) ;
	    if (f) {
	        cchar	*fmt ;

	        if (pip->tardname == NULL) {
	            if ((cp = getenv(VARTARDNAME)) != NULL) {
	                pip->have.tardname = TRUE ;
	                pip->tardname = cp ;
	            }
	        }

	        if ((pip->tardname == NULL) || (pip->tardname[0] == '\0')) {
	            fmt ="%s: no target directory specified\n" ;
	            rs = SR_INVALID ;
	            bprintf(pip->efp,fmt, pip->progname) ;
	        }
	        if (rs >= 0) {
	            rs = fsdirtreestat(pip->tardname,0,&pip->tarstat) ;
	            if ((rs < 0) || (! S_ISDIR(pip->tarstat.st_mode))) {
	                fmt = "%s: target directory inaccessible (%d)\n" ;
	                rs = SR_NOTDIR ;
	                bprintf(pip->efp,fmt, pip->progname,rs) ;
	            }
	        }
	    }
	} /* end block */
	if (rs < 0) ex = EX_USAGE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: tardname=%s\n",pip->tardname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: tardname=%s\n",pip->progname,pip->tardname) ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: follow=%u\n",pip->progname,pip->f.follow) ;
#endif

	if ((rs >= 0) && (pip->nice > 0)) {
	    int	n = MIN(pip->nice,19) ;
	    rs = u_nice(n) ;
	}

/* get ready */

	if (rs >= 0) {
	    if (paramopt_havekey(&pip->aparams,po_sufreq) >= 0) {
	        pip->have.sufreq = TRUE ;
	        if (pip->debuglevel > 0)
	            rs = procprintsufs(pip,po_sufreq) ;
	    } /* end if */
	} /* end if */

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procsuf_begin(pip)) >= 0) {
	        if ((rs = procuniq_begin(pip)) >= 0) {
	            if ((rs = procprune_begin(pip,pfname)) >= 0) {

#if	CF_DIRS
	            if ((rs >= 0) && (pip->f.follow || pip->f.f_uniq)) {
	                rs = procdir_begin(pip) ;
	            }
#endif /* CF_DIRS */

	            if (rs >= 0) {
	                switch (pip->progmode) {
	                case progmode_filesyncer:
	                    rs = proclink_begin(pip) ;
	                    break ;
	                case progmode_filerm:
	                    rs = procrm_begin(pip) ;
	                    break ;
	                } /* end switch */
	            } /* end if */

	            if (rs >= 0) {
	                cchar	*ofn = ofname ;
	                cchar	*afn = afname ;
	                rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
	            }

	            {
	                switch (pip->progmode) {
	                case progmode_filesyncer:
	                    rs1 = proclink_end(pip) ;
	                    break ;
	                case progmode_filerm:
	                    rs1 = procrm_end(pip) ;
	                    break ;
	                } /* end switch */
	                if (rs >= 0) rs = rs1 ;
	            } /* end block */

#if	CF_DIRS
	            if (pip->f.follow || pip->f.f_uniq) {
	                rs1 = procdir_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            }
#endif /* CF_DIRS */

	                rs1 = procprune_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procprune) */
	            rs1 = procuniq_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuniq) */
	        rs1 = procsuf_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procsuf) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;
	}

#ifdef	COMMENT
	if ((rs >= 0) && (pan == 0) && (! pip->f.zargs) && (! pip->f.quiet)) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no files or directories were specified\n",
	        pip->progname) ;
	}
#endif /* COMMENT */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid usage (%d)\n",
	                pip->progname,rs) ;
	        }
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = TRUE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&pip->aparams) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&pip->akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        }
	        ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<dir(s)> ...] [-af {<afile>|-}] \n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-f] [-c] [-ia] [-s <suffix(es)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-sa <sufacc(s)>] [-sr <sufrej(s)>] [-t <type(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-o <option(s)] [-prune <name(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip)
{
	KEYOPT		*kop = &pip->akopts ;
	KEYOPT_CUR	kcur ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        int	v ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,1,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case progopt_uniq:
	                    if (! pip->final.f_uniq) {
	                        pip->final.f_uniq = TRUE ;
	                        pip->have.f_uniq = TRUE ;
	                        pip->f.f_uniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_uniq = (rs > 0) ;
	                        }
	                    }
	                    break ;
/* [what is this?] */
	                case progopt_name:
	                    if (! pip->final.f_name) {
	                        pip->final.f_name = TRUE ;
	                        pip->f.f_name = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_name = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_noprog:
	                    if (! pip->final.f_noprog) {
	                        pip->final.f_noprog = TRUE ;
	                        pip->f.f_noprog = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_noprog = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_nosock:
	                    if (! pip->final.f_nosock) {
	                        pip->final.f_nosock = TRUE ;
	                        pip->f.f_nosock = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_nosock = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_nopipe:
	                case progopt_nofifo:
	                    if (! pip->final.f_nopipe) {
	                        pip->final.f_nopipe = TRUE ;
	                        pip->f.f_nopipe = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_nopipe = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_nodev:
	                    if (! pip->final.f_nodev) {
	                        pip->final.f_nodev = TRUE ;
	                        pip->f.f_nodev = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_nodev = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_noname:
	                    if (! pip->final.f_noname) {
	                        pip->final.f_noname = TRUE ;
	                        pip->f.f_noname = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_noname = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_nolink:
	                    if (! pip->final.f_nolink) {
	                        pip->final.f_nolink = TRUE ;
	                        pip->f.f_nolink= TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_nolink = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_noextra:
	                    if (! pip->final.f_noextra) {
	                        pip->final.f_noextra = TRUE ;
	                        pip->f.f_noextra = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_noextra = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_nodotdir:
	                    if (! pip->final.f_nodotdir) {
	                        pip->final.f_nodotdir = TRUE ;
	                        pip->f.f_nodotdir = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.f_nodotdir = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_cores:
	                    if (! pip->final.cores) {
	                        pip->final.cores = TRUE ;
	                        pip->f.cores = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.cores = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_s:
	                    if ((vl > 0) && (! pip->final.sufreq)) {
	                        rs = procsuf_load(pip,suf_req,vp,vl) ;
	                    }
	                    break ;
	                case progopt_sa:
	                    if ((vl > 0) && (! pip->final.sufacc)) {
	                        rs = procsuf_load(pip,suf_acc,vp,vl) ;
	                    }
	                    break ;
	                case progopt_sr:
	                case progopt_nosuf:
	                    if ((vl > 0) && (! pip->final.sufrej)) {
	                        rs = procsuf_load(pip,suf_rej,vp,vl) ;
	                    }
	                    break ;
	                case progopt_prune:
	                    if (vl > 0) {
	                        PARAMOPT	*pop = &pip->aparams ;
	                        cchar		*po = po_prune ;
	                        rs = paramopt_loads(pop,po,vp,vl) ;
				pip->f.prune |= (rs > 0) ;
	                    }
	                    break ;
	                case progopt_younger:
	                case progopt_yi:
	                    if ((vl > 0) && (! pip->final.younger)) {
	                        pip->final.younger = TRUE ;
	                        pip->have.younger = TRUE ;
	                        rs = cfdecti(vp,vl,&v) ;
	                        pip->younger = v ;
	                    }
	                    break ;
	                case progopt_nice:
	                    if ((vl > 0) && (! pip->final.nice)) {
	                        pip->final.nice = TRUE ;
	                        pip->have.nice = TRUE ;
	                        rs = optvalue(vp,vl) ;
	                        pip->nice = rs ;
	                    }
	                    break ;
	                case progopt_follow:
	                    if (! pip->final.follow) {
	                        pip->final.follow = TRUE ;
	                        pip->have.follow = TRUE ;
	                        pip->f.follow = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.follow = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_iacc:
	                case progopt_ignacc:
	                    if (! pip->final.iacc) {
	                        pip->final.iacc = TRUE ;
	                        pip->f.iacc = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.iacc = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_im:
	                    if (! pip->final.im) {
	                        pip->final.im = TRUE ;
	                        pip->f.im = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.im = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_quiet:
	                    if (! pip->final.quiet) {
	                        pip->final.quiet = TRUE ;
	                        pip->have.quiet = TRUE ;
	                        pip->f.quiet = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.quiet = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_ff:
	                    if (! pip->final.ff) {
	                        pip->final.ff = TRUE ;
	                        pip->have.ff = TRUE ;
	                        pip->f.ff = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.ff = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_readable:
	                    if (! pip->final.readable) {
	                        pip->final.readable = TRUE ;
	                        pip->have.readable = TRUE ;
	                        pip->f.readable = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.readable = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case progopt_older:
	                    if (! pip->final.older) {
	                        pip->final.older = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->older = v ;
	                        }
	                    }
	                    break ;
	                case progopt_accessed:
	                    if (! pip->final.accessed) {
	                        pip->final.accessed = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            pip->accessed = v ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
	                rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopts-cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procfts(PROGINFO *pip)
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		vl ;
	int		fti ;
	int		c = 0 ;
	cchar		*varftypes = getenv(VARFTYPES) ;
	cchar		*vp ;

	if (varftypes != NULL) {
	    rs = paramopt_loads(pop,po_fts,varftypes,-1) ;
	}

	if (rs >= 0) {
	    if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	        while (rs >= 0) {

	            vl = paramopt_fetch(pop,po_fts,&cur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            if (vl == 0) continue ;
	            rs = vl ;
	            if (rs < 0) break ;

	            fti = matostr(ftstrs,1,vp,vl) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procfts: v=%t\n",vp,vl) ;
#endif

	            if (fti >= 0) {
	                switch (fti) {
	                case ftstr_regular:
	                case ftstr_file:
	                    pip->fts |= (1 << ft_r) ;
	                    break ;
	                case ftstr_directory:
	                    pip->fts |= (1 << ft_d) ;
	                    break ;
	                case ftstr_block:
	                    pip->fts |= (1 << ft_b) ;
	                    break ;
	                case ftstr_character:
	                    pip->fts |= (1 << ft_c) ;
	                    break ;
	                case ftstr_pipe:
	                case ftstr_fifo:
	                    pip->fts |= (1 << ft_p) ;
	                    break ;
	                case ftstr_socket:
	                    pip->fts |= (1 << ft_s) ;
	                    break ;
	                case ftstr_link:
	                    pip->fts |= (1 << ft_l) ;
	                    break ;
	                case ftstr_name:
	                    pip->fts |= (1 << ft_n) ;
	                    break ;
	                case ftstr_door:
	                    pip->fts |= (1 << ft_D) ;
	                    break ;
	                case ftstr_exists:
	                    pip->fts |= (1 << ft_e) ;
	                    break ;
	                } /* end switch */
	                c += 1 ;
	            } /* end if */

	        } /* end while */

	        paramopt_curend(pop,&cur) ;
	    } /* end if (cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn= BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;
	    pip->ofp = ofp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procname(pip,cp) ;
	                    if (rs < 0) {
	                        fmt = "%s: error name=%s (%d)\n" ;
	                        bprintf(pip->efp,fmt,pn,cp,rs) ;
	                    }
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for (looping through requested circuits) */
	    } /* end if (ok) */

/* process any files in the argument filename list file */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        lbuf[((cp-lbuf)+cl)] = '\0' ;
	                        rs = procname(pip,cp) ;
	                    }
	                }

	                if (rs < 0) {
	                    bprintf(pip->efp,
	                        "%s: error name=%s (%d)\n",
	                        pip->progname,cp,rs) ;
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                fmt = "%s: inacessible argument-list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pip->progmode == progmode_filesize)) {
	        if ((pip->verboselevel > 0) && (! pip->f.zargs)) {
	            long	blocks, blockbytes ;

	            fmt = "%lu megabyte%s and %lu bytes\n" ;
	            bprintf(ofp,fmt,
	                pip->megabytes,
	                ((pip->megabytes == 1) ? "" : "s"),
	                pip->bytes) ;

/* calculate UNIX blocks */

	            blocks = pip->megabytes * 1024 * 2 ;
	            blocks += (pip->bytes / UNIXBLOCK) ;
	            blockbytes = (pip->bytes % UNIXBLOCK) ;

	            fmt = "%lu UNIX� blocks and %lu bytes\n" ;
	            bprintf(ofp,fmt,blocks,blockbytes) ;

	        } /* end if (verbosity allowed) */
	    } /* end if (program mode=filesize) */

	    pip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return rs ;
}
/* end subroutine (procargs) */


int procname(PROGINFO *pip,cchar name[])
{
	FSDIRTREE_STAT	sb, ssb, *sbp = &sb ;
	int		rs = SR_OK ;
	int		f_go = TRUE ;
	int		f_islink = FALSE ;
	int		f_isdir = FALSE ;
	char		tmpfname[MAXPATHLEN+1] ;

	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: ent name=%s\n",name) ;
#endif

	if ((name[0] == '/') && (name[1] == 'u')) {
	    rs = mkuserpath(tmpfname,NULL,name,-1) ;
	    if (rs > 0) name = tmpfname ;
	}

	if (rs >= 0)
	    rs = fsdirtreestat(name,1,&sb) ; /* this is LSTAT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    if (rs < 0) sb.st_mode = 0 ;
	    debugprintf("main/procname: fsdirtreestat() rs=%d mode=%0o\n", 
	        rs,sb.st_mode) ;
	}
#endif

	if (pip->f.im && isNotPresent(rs)) {
	    rs = SR_OK ;
	    f_go = FALSE ;
	}

	f_isdir = S_ISDIR(sb.st_mode) ;
	f_islink = S_ISLNK(sb.st_mode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procname: f_isdir=%u\n",f_isdir) ;
	    debugprintf("main/procname: f_islink=%u\n",f_islink) ;
	}
#endif

	if ((rs >= 0) && f_go && f_islink && (pip->f.follow || pip->f.ff)) {

	    if ((rs = fsdirtreestat(name,0,&ssb)) >= 0) { /* STAT */
	        f_isdir = S_ISDIR(ssb.st_mode) ;
	        f_islink = S_ISLNK(sb.st_mode) ;
	        sbp = &ssb ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    } else if (rs == SR_LOOP) {
		rs = SR_OK ;
		if (! pip->f.quiet) {
		    cchar	*pn = pip->progname ;
		    cchar	*fmt ;
		    fmt = "%s: symbolic link exhaustion\n" ;
		    bprintf(pip->efp,fmt,pn) ;
		    fmt = "%s: name=%s\n" ;
		    bprintf(pip->efp,fmt,pn,name) ;
		}
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procname: symlink dangle=%u\n",
	            ((rs < 0)?1:0)) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_go) {
	    if (pip->f.readable && (! f_islink)) {
	        if ((rs = uc_access(name,R_OK)) < 0) {
	            f_go = FALSE ;
	            if (rs == SR_ACCESS) rs = SR_OK ;
	        }
	    }
	    if ((rs >= 0) && f_go && pip->f.prune) {
		rs = procprune(pip,name) ;
		f_go = (rs > 0) ;
	    }
	    if ((rs >= 0) && f_go) {
	        if (f_isdir) {
	            rs = procdir(pip,name,sbp) ;
	        } else {
	            rs = procother(pip,name,sbp) ;
	        }
	    } /* end if */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procname) */


static int procdir(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl = strlen(name) ;
	int		opts ;
	int		size ;
	int		c = 0 ;
	char		*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: name=%s\n",name) ;
#endif

	while ((nl > 0) && (name[nl-1] == '/')) nl -= 1 ;

	if (pip->f.f_nodotdir) {
	    if (name[0] == '.') goto ret0 ;
	}

/* optionally register ourselves */

#if	CF_DIRS
	if (pip->f.follow || pip->f.f_uniq) {
	    dev_t	dev = sbp->st_dev ;
	    uino_t	ino = sbp->st_ino ;
	    int		f = TRUE ;
	    if ((rs = procdir_have(pip,dev,ino,name,nl)) == 0) {
	        f = FALSE ;
	        if ((rs = procdir_haveprefix(pip,name,nl)) > 0) {
	            f = TRUE ;
	            rs = procdir_addprefix(pip,name,nl) ;
	        }
	    }
	    if ((rs >= 0) && f) goto ret0 ;
	}
	if (rs < 0) goto ret0 ;
#endif /* CF_DIRS */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: 2 rs=%d\n",rs) ;
#endif

/* do all of our children */

	size = (nl + 1 + MAXPATHLEN + 1) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    FSDIRTREE		d ;
	    FSDIRTREE_STAT	fsb ;
	    char		*fname = p ;
	    char		*bp ;

	    bp = strwcpy(fname,name,nl) ;
	    *bp++ = '/' ;

	    opts = 0 ;
	    if (pip->f.follow) {
	        opts |= FSDIRTREE_MFOLLOW ;
	        opts |= FSDIRTREE_MUNIQDIR ;
	    }
	    if (pip->f.f_uniq) {
	        opts |= FSDIRTREE_MUNIQDIR ;
	    }

	    if ((rs = fsdirtree_open(&d,name,opts)) >= 0) {
	        const int	mpl = MAXPATHLEN ;

		if (pip->f.prune) {
		    rs = fsdirtree_prune(&d,pip->prune) ;
		}

		if (rs >= 0) {
	        while ((rs = fsdirtree_read(&d,&fsb,bp,mpl)) > 0) {
		    int	f_go = TRUE ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procdir: direntry=%s\n",bp) ;
#endif

	            if (pip->debuglevel >= 2) {
	                bprintf(pip->efp,"%s: looking fn=%s (%d)\n",
	                    pip->progname,fname,rs) ;
	            }

	            if ((rs >= 0) && f_go) {
	                c += 1 ;
	                rs = procother(pip,fname,&fsb) ;
	                if (rs >= 0) c += 1 ;
	            }

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procdir: while-bot rs=%d\n",rs) ;
#endif

	            if (rs < 0) break ;
	        } /* end while (reading entries) */
		} /* end if (ok) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procdir: while-out rs=%d\n",rs) ;
#endif
	        rs1 = fsdirtree_close(&d) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (fsdirtree) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procdir: dir-out rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
	        if (isAccess(rs) && pip->f.iacc) rs = SR_OK ;
	        if ((! pip->f.quiet) && (rs < 0)) {
	            bprintf(pip->efp,"%s: dname=%s (%d)\n",
	                pip->progname,name,rs) ;
		}
	        if ((! istermrs(rs)) && pip->f.nostop) rs = SR_OK ;
	    }

	    rs1 = uc_free(p) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: 4 rs=%d\n",rs) ;
#endif

/* do ourself */

	if (rs >= 0) {
	    c += 1 ;
	    rs = procother(pip,name,sbp) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdir) */


static int procother(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	FILEINFO	ck, *ckp = &ck ;
#if	CF_FOLLOWFILES
	FSDIRTREE_STAT	ssb ;
#endif
	int		rs = SR_OK ;
	int		rs1 ;
	int		bnl = 0 ;
	int		f_process = FALSE ;
	int		f_accept = FALSE ;
	int		f_islink = FALSE ;
	int		f_suf ;
	cchar		*bnp ;

	memset(ckp,0,sizeof(FILEINFO)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    cchar	*cp ;
	    debugprintf("main/procother: ent filepath=%s\n",name) ;
	    if (S_ISLNK(sbp->st_mode)) cp = "LINK" ;
	    else if (S_ISDIR(sbp->st_mode)) cp = "DIR" ;
	    else if (S_ISREG(sbp->st_mode)) cp = "REG" ;
	    else cp = "OTHER" ;
	    debugprintf("main/procother: filetype=%s\n",cp) ;
	}
#endif /* CF_DEBUG */

	pip->c_files += 1 ;
	if (sbp->st_ctime == 0)
	    goto ret0 ;

/* fill in some information */

#if	CF_FTCASE
	{
	    const int	ftype = (sbp->st_mode & S_IFMT) ;
	    int		fts = 0 ;
	    switch (ftype) {
	    case S_IFIFO:
	        fts |= (1 << ft_p) ;
	        break ;
	    case S_IFCHR:
	        fts |= (1 << ft_c) ;
	        break ;
	    case S_IFDIR:
	        fts |= (1 << ft_d) ;
	        break ;
	    case S_IFNAM:
	        fts |= (1 << ft_n) ;
	        break ;
	    case S_IFBLK:
	        fts |= (1 << ft_b) ;
	        break ;
	    case S_IFREG:
	        fts |= (1 << ft_r) ;
	        break ;
	    case S_IFLNK:
	        fts |= (1 << ft_l) ;
	        break ;
	    case S_IFSOCK:
	        fts |= (1 << ft_s) ;
	        break ;
	    case S_IFDOOR:
	        fts |= (1 << ft_D) ;
	        break ;
	    } /* end switch */
	    ckp->fts = fts ;
	}
#else /* CF_FTCASE */
	{
	    register uint	ft = 0 ;
	    if (S_ISREG(sbp->st_mode)) ft |= (1 << ft_r) ;
	    else if (S_ISDIR(sbp->st_mode)) ft |= (1 << ft_d) ;
	    else if (S_ISBLK(sbp->st_mode)) ft |= (1 << ft_b) ;
	    else if (S_ISCHR(sbp->st_mode)) ft |= (1 << ft_c) ;
	    else if (S_ISFIFO(sbp->st_mode)) ft |= (1 << ft_p) ;
	    else if (S_ISLNK(sbp->st_mode)) ft |= (1 << ft_l) ;
	    else if (S_ISSOCK(sbp->st_mode)) ft |= (1 << ft_s) ;
	    else if (S_ISNAM(sbp->st_mode)) ft |= (1 << ft_n) ;
	    else if (S_ISDOOR(sbp->st_mode)) ft |= (1 << ft_D) ;
	    ckp->fts = ft ;
	}
#endif /* CF_FTCASE */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	i ;
	    for (i = 0 ; i < ft_overlast ; i += 1) {
	        if (BATSTI(&ckp->fts,i))
	            debugprintf("main/procother: ft[%u]=TRUE\n",i) ;
	    }
	}
#endif /* CF_DEBUG */

/* check age */

	if (pip->younger > 0) {
	    if ((pip->daytime - sbp->st_mtime) >= pip->younger)
	        goto done ;
	}

	if (pip->older > 0) {
	    if ((pip->daytime - sbp->st_mtime) < pip->older)
	        goto done ;
	}

	if (pip->accessed > 0) {
	    if ((pip->daytime - sbp->st_atime) < pip->accessed)
	        goto done ;
	}

/* if this is a file link, see if it is a directory */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: symbolic link check?\n") ;
#endif

	if (S_ISLNK(sbp->st_mode)) {
	    f_islink = TRUE ;

	    if (pip->f.f_nolink)
	        goto done ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procother: symbolic_link YES\n") ;
#endif

#if	CF_FOLLOWFILES
	    if (pip->f.follow) {

	        sbp = &ssb ;
	        rs = fsdirtreestat(name,0,&ssb) ; /* STAT */
	        f_islink = S_ISLNK(ssb.st_mode) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: fsdirtreestat() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: symlink DANGLING\n") ;
#endif
	            if (! pip->f.nostop) goto done ;
	            rs = SR_OK ;
	        }

	    } /* end if (follow link) */
#endif /* CF_FOLLOWFILES */

	} /* end if (symbolic-link-file) */

/* if types were specified, is this file one of them? (if not return now) */

	if ((pip->fts > 0) && ((ckp->fts & pip->fts) == 0))
	    goto done ;

	if ((pip->fnos > 0) && ((ckp->fts & pip->fnos) != 0))
	    goto done ;

	if (pip->f.f_uniq) {
	    dev_t	dev = sbp->st_dev ;
	    uino_t	ino = sbp->st_ino ;
	    if ((rs = procuniq_have(pip,dev,ino)) > 0) goto done ;
	}

#if	CF_DIRS
	if (pip->f.follow || pip->f.f_uniq) {
	    const int	nl = strlen(name) ;
	    int		f = TRUE ;
	    if ((rs = procdir_haveprefix(pip,name,nl)) >= 0) {
	        f = (rs > 0) ;
	        if (S_ISDIR(sbp->st_mode)) {
	            dev_t	dev = sbp->st_dev ;
	            uino_t	ino = sbp->st_ino ;
	            if ((rs = procdir_have(pip,dev,ino,name,nl)) > 0) {
	                f = TRUE ;
	            }
	        } /* end if (is-directory) */
	        if ((rs >= 0) && f)
	            rs = procdir_addprefix(pip,name,nl) ;
	    } /* end if (does not have disallowed prefix) */
	    if ((rs >= 0) && f) goto done ;
	} /* end if (uniqueness check) */
	if (rs < 0) goto done ;
#endif /* CF_DIRS */

/* prepare for suffix checks */

	f_suf = (pip->have.sufreq || pip->f.sufacc || pip->f.sufrej) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: f_suf=%u\n",f_suf) ;
#endif

	if (f_suf || pip->f.cores) {
	    bnl = sfbasename(name,-1,&bnp) ;
	}

	if (f_suf) {
	    if (bnl <= 0) f_suf = FALSE ;
	    if (f_suf && (bnl > 0) && (bnp[0] == '.')) {

	        if ((bnl == 1) ||
	            ((bnl == 2) && (bnp[1] == '.'))) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: name was a dotter\n") ;
#endif

	            goto done ;
	        }

	    } /* end if */
	} /* end if (funny name check) */

/* check if it has a suffix already */

	f_process = TRUE ;
	if (f_suf) {
	    VECPSTR	*slp ;
	    int		sl ;
	    cchar	*tp, *sp ;
	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {
	        sp = (tp+1) ;
	        sl = ((bnp+bnl)-(tp+1)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: suf=%t \n",sp,sl) ;
#endif

/* check against the suffix-required list */

	        if (pip->have.sufreq) {
	            rs = procsuf_have(pip,sp,sl) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: procsuf_have() rs=%d\n",
	                    rs) ;
#endif

	            if (rs == 0) {
	                f_process = FALSE ;
	            } else {
	                f_accept = TRUE ;
	            }
	        } /* end if */

/* check against the suffix-acceptance list */

	        if (f_process && pip->f.sufacc && (! f_accept)) {
	            slp = (pip->sufs + suf_acc) ;
	            rs1 = vecpstr_findn(slp,sp,sl) ;
	            f_accept = (rs1 >= 0) ;
	        }

/* check against the suffix-rejectance list */

	        if (f_process && pip->f.sufrej && (! f_accept)) {
	            slp = (pip->sufs + suf_rej) ;
	            rs1 = vecpstr_findn(slp,sp,sl) ;
	            if (rs1 >= 0) f_process = FALSE ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procother: suf fa=%u f_process=%u\n",
	                f_accept,f_process) ;
#endif

	    } else {
	        f_suf = FALSE ;
	        if (pip->have.sufreq) f_process = FALSE ;
	    }
	} /* end if (suffix lists) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procother: mid rs=%d f_suf=%u f_process=%u\n",
	        rs,f_suf,f_process) ;
#endif

/* check if it is a program (and disallowed) */

	if ((rs >= 0) && f_process && pip->f.f_noprog && (! f_accept)) {
	    if (S_ISREG(sbp->st_mode)) {
	        rs = fileobject(name) ;
	        f_process = (rs == 0) ;
	    }
	} /* end if (no-program) */

/* check if it is a program (and required) */

	if ((rs >= 0) && f_process && pip->f.cores && (! f_accept)) {
	    f_process = FALSE ;
	    if (S_ISREG(sbp->st_mode) && (strwcmp("core",bnp,bnl) == 0)) {
	        rs = fileobject(name) ;
	        f_process = (rs > 0) ;
	    }
	} /* end if (yes-program) */

	if ((rs >= 0) && f_process && pip->f.readable && (! f_islink)) {
	    if ((rs = uc_access(name,R_OK)) < 0) {
	        f_process = FALSE ;
	        if (rs == SR_ACCESS) rs = SR_OK ;
	    }
	}

/* process this file */

	if ((rs >= 0) && f_process) {

	    pip->c_processed += 1 ;
	    switch (pip->progmode) {

	    case progmode_filesize:
	        rs = procsize(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filefind:
	        {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procother: name=>%s<\n",name) ;
#endif
	            if (! pip->f.quiet)
	                rs = bprintf(pip->ofp,"%s\n",name) ;
	        }
	        break ;

	    case progmode_filelinker:
	        rs = proclink(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filesyncer:
	        rs = procsync(pip,name,sbp,ckp) ;
	        break ;

	    case progmode_filerm:
	        rs = procrm(pip,name,sbp,ckp) ;
	        break ;

	    default:
	        rs = SR_NOANODE ;
	        break ;

	    } /* end switch */

	} /* end if (processed) */

done:
	if ((pip->debuglevel > 0) || ((rs < 0) && (! pip->f.quiet))) {
	    if ((! isAccess(rs)) || (! pip->f.iacc)) {
	        bprintf(pip->efp,"%s: fname=%s (%d:%u)\n",
	            pip->progname,name,rs,f_process) ;
	    }
	}

	if (rs < 0) {
	    if (isAccess(rs) && pip->f.iacc) rs = SR_OK ;
	    if ((! istermrs(rs)) && pip->f.nostop) rs = SR_OK ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procother: ret rs=%d f_process=%u\n",
	        rs,f_process) ;
#endif

	return (rs >= 0) ? f_process : rs ;
}
/* end subroutine (procother) */


#if	CF_NEWPRUNE
static int procprune(PROGINFO *pip,cchar *name)
{
	int		rs = SR_OK ;
	int		f_go = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procprune: ent name=%s\n",name) ;
#endif

	if (pip->prune != NULL) {
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = sfbasename(name,-1,&cp)) > 0) {
	        f_go = (matstr(pip->prune,cp,cl) < 0) ;
	    }
	} /* end if (basename) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procprune: ret rs=%d f_go=%u\n",rs,f_go) ;
#endif

	return (rs >= 0) ? f_go : rs ;
}
/* end subroutine (procprune) */
#else /* CF_NEWPRUNE */
static int procprune(PROGINFO *pip,cchar *name)
{
	int		rs = SR_OK ;
	int		cl ;
	int		f_go = TRUE ;
	cchar		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procprune: ent name=%s\n",name) ;
#endif

	if ((cl = sfbasename(name,-1,&cp)) > 0) {
	    PARAMOPT		*pop = &pip->aparams ;
	    PARAMOPT_CUR	cur ;
	    if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	        int	vl ;
	        cchar	*po = po_prune ;
	        cchar	*vp ;
	        while (rs >= 0) {
	            vl = paramopt_fetch(pop,po,&cur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main/procprune: cl=%u vl=%u\n",cl,vl) ;
	debugprintf("main/procprune: c=%t\n",cp,cl) ;
	debugprintf("main/procprune: v=%t\n",vp,vl) ;
	}
#endif
	            if ((rs >= 0) && (cl == vl) && (cp[0] == vp[0])) {
	                f_go = (strwcmp(cp,vp,vl) != 0) ;
	                if (! f_go) break ;
	            }
	        } /* end while */
	        paramopt_curend(pop,&cur) ;
	    } /* end if (paramopt-cur) */
	} /* end if (basename) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procprune: ret rs=%d f_go=%u\n",rs,f_go) ;
#endif

	return (rs >= 0) ? f_go : rs ;
}
/* end subroutine (procprune) */
#endif /* CF_NEWPRUNE */


static int procprintfts(PROGINFO *pip,cchar *po)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (pip->debuglevel > 0) {
	    PARAMOPT		*pop = &pip->aparams ;
	    PARAMOPT_CUR	cur ;
	    if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	        int	vl ;
	        cchar	*pn = pip->progname ;
	        cchar	*vp ;

	        while ((vl = paramopt_fetch(pop,po,&cur,&vp)) >= 0) {
	            rs1 = bprintf(pip->efp,"%s: ft=%t\n",pn,vp,vl) ;
	            if (rs1 > 0) wlen += rs1 ;
	            if (rs1 < 0) break ;
	        } /* end while */

	        paramopt_curend(pop,&cur) ;
	    } /* end if (paramopt-cur) */
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintfts) */


static int procprintsufs(PROGINFO *pip,cchar *po)
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (pip->debuglevel > 0) {
	    if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	        int	vl ;
	        cchar	*pn = pip->progname ;
	        cchar	*vp ;

	        while ((vl = paramopt_fetch(pop,po,&cur,&vp)) >= 0) {
	            rs1 = bprintf(pip->efp,"%s: suf=%t\n",pn,vp,vl) ;
	            if (rs1 > 0) wlen += rs1 ;
	            if (rs1 < 0) break ;
	        } /* end while */

	        paramopt_curend(pop,&cur) ;
	    } /* end if (cursor) */
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintsufs) */


static int procsuf_have(PROGINFO *pip,cchar *sp,int sl)
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		f = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	    int		vl ;
	    int		m ;
	    cchar	*key = po_sufreq ;
	    cchar	*vp ;

	    while (rs >= 0) {
	        vl = paramopt_fetch(pop,key,&cur,&vp) ;
	        if (vl == SR_NOTFOUND) break ;
	        rs = vl ;
	        if ((rs >= 0) && (sl == vl)) {
	            m = nleadstr(sp,vp,vl) ;
	            f = (m == vl) ;
	            if (f) break ;
	        }
	    } /* end while */

	    paramopt_curend(pop,&cur) ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procsuf_have) */


static int procsuf_begin(PROGINFO *pip)
{
	PARAMOPT	*pop = &pip->aparams ;
	VECPSTR		*vlp ;
	int		rs = SR_OK ;
	int		si ;
	int		c = 0 ;
	cchar		*po = NULL ;

	for (si = 0 ; si < suf_overlast ; si += 1) {
	    int	f = FALSE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsuf_begin: si=%u c=%u\n",si,c) ;
#endif

	    switch (si) {
	    case suf_req:
	        f = pip->have.sufreq ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    case suf_acc:
	        f = pip->have.sufacc ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    case suf_rej:
	        f = pip->have.sufrej ;
	        if (f) {
	            po = sufs[si] ;
	            vlp = (pip->sufs + si) ;
	        }
	        break ;
	    } /* end switch */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsuf_begin: si=%u f=%u\n",si,f) ;
#endif

	    if (f) {
	        int n = paramopt_countvals(pop,po) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procsuf_begin: si=%u n=%u\n",si,n) ;
#endif

	        if (n > 0) {
	            PARAMOPT_CUR	cur ;
	            if ((rs = vecpstr_start(vlp,n,0,0)) >= 0) {
	                switch (si) {
	                case suf_req:
	                    pip->open.sufreq = TRUE ;
	                    break ;
	                case suf_acc:
	                    pip->open.sufacc = TRUE ;
	                    break ;
	                case suf_rej:
	                    pip->open.sufrej = TRUE ;
	                    break ;
	                } /* end switch */
	                if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	                    int	vl ;
	                    cchar	*vp ;
	                    while (rs >= 0) {
	                        vl = paramopt_fetch(pop,po,&cur,&vp) ;
	                        if (vl == SR_NOTFOUND) break ;
	                        rs = vl ;
	                        if ((rs >= 0) && (vl > 0)) {
	                            rs = vecpstr_adduniq(vlp,vp,vl) ;
	                            if (rs < INT_MAX) c += 1 ;

#if	CF_DEBUG
	                            if (DEBUGLEVEL(3) && (rs < INT_MAX))
	                                debugprintf("main/procsuf_begin: "
	                                    "suf=%t\n",
	                                    vp,vl) ;
#endif

	                        }
	                    } /* end while */
	                    paramopt_curend(pop,&cur) ;
	                } /* end if */
	                if (c > 0) {
	                    switch (si) {
	                    case suf_req:
	                        pip->f.sufreq = TRUE ;
	                        break ;
	                    case suf_acc:
	                        pip->f.sufacc = TRUE ;
	                        break ;
	                    case suf_rej:
	                        pip->f.sufrej = TRUE ;
	                        break ;
	                    } /* end switch */
	                } /* end if */
	            } /* end if (vecpstr_start) */
	        } /* end if (n) */
	    } /* end if */

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsuf_begin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsuf_begin) */


static int procsuf_end(PROGINFO *pip)
{
	VECPSTR		*slp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		f ;

	for (i = 0 ; i < suf_overlast ; i += 1) {
	    slp = (pip->sufs + i) ;
	    f = FALSE ;
	    switch (i) {
	    case suf_req:
	        f = pip->open.sufreq ;
	        pip->open.sufreq = FALSE ;
	        break ;
	    case suf_acc:
	        f = pip->open.sufacc ;
	        pip->open.sufacc = FALSE ;
	        break ;
	    case suf_rej:
	        f = pip->open.sufrej ;
	        pip->open.sufrej = FALSE ;
	        break ;
	    } /* end switch */
	    if (f) {
	        rs1 = vecpstr_finish(slp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (procsuf_end) */


static int procsuf_load(PROGINFO *pip,int si,cchar *ap,int al)
{
	PARAMOPT	*pop = &pip->aparams ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*po ;
	cchar		*var ;

#if	CF_DEBUGS
	if (si < suf_overlast)
	    debugprintf("main/procsuf_load: suf=%s(%u)\n",sufs[si],si) ;
#endif

	if (ap != NULL) {

	    switch (si) {
	    case suf_req:
	        po = po_sufreq ;
	        var = VARSUFREQ ;
	        break ;
	    case suf_acc:
	        po = po_sufacc ;
	        var = VARSA ;
	        break ;
	    case suf_rej:
	        po = po_sufrej ;
	        var = VARSR ;
	        break ;
	    default:
	        rs = SR_NOANODE ;
	        break ;
	    } /* end switch */

	    if (rs >= 0) {
	        if (strwcmp("-",ap,al) != 0) {
	            if (strwcmp("+",ap,al) == 0) {
	                ap = getenv(var) ;
	                al = -1 ;
	            }
	            if (ap != NULL) {
	                rs = paramopt_loads(pop,po,ap,al) ;
	                c = rs ;

#if	CF_DEBUGS
	                debugprintf("main/procsuf_load: "
	                    "paramopt_loads() rs=%d\n",
	                    rs) ;
#endif

	                if ((rs >= 0) && (c > 0)) {
	                    switch (si) {
	                    case suf_req:
	                        pip->have.sufreq = TRUE ;
	                        break ;
	                    case suf_acc:
	                        pip->have.sufacc = TRUE ;
	                        break ;
	                    case suf_rej:
	                        pip->have.sufrej = TRUE ;
	                        break ;
	                    } /* end switch */
	                }
	            }
	        } /* end if */
	    } /* end if (ok) */

	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("main/procsuf_load: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsuf_load) */


static int procrm_begin(PROGINFO *pip)
{
	int		rs ;

	rs = vecpstr_start(&pip->rmdirs,0,0,0) ;
	pip->f.rmdirs = (rs >= 0) ;

	return rs ;
}
/* end subroutine (procrm_begin) */


static int procrm_add(PROGINFO *pip,cchar *dp,int dl)
{
	int		rs ;

	if (dl < 0) dl = strlen(dp) ;

	if ((dl > 0) && (dp[dl-1] == '/')) dl -=1 ;

	rs = vecpstr_add(&pip->rmdirs,dp,dl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procrm_add: n=%t\n",dp,dl) ;
#endif

	return rs ;
}
/* end subroutine (procrm_add) */


static int procrm_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.rmdirs) {
	    VECPSTR	*rlp = &pip->rmdirs ;
	    int		i ;
	    cchar	*np ;
	    pip->f.rmdirs = FALSE ;
	    vecpstr_sort(rlp,vcmprstr) ;
	    for (i = 0 ; vecpstr_get(rlp,i,&np) >= 0 ; i += 1) {
	        rs1 = u_rmdir(np) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procrm_end: n=%s rs=%d\n",np,rs1) ;
#endif
	        if (rs >= 0) rs = rs1 ;
	    } /* end for */
	    rs1 = vecpstr_finish(&pip->rmdirs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (rmdirs) */

	return rs ;
}
/* end subroutine (procrm_end) */


static int procdir_begin(PROGINFO *pip)
{
	HDB		*dbp = &pip->dirs ;
	const int	n = 50 ;
	const int	at = 1 ;	/* use 'lookaside(3dam)' */
	int		rs ;

	if ((rs = hdb_start(dbp,n,at,diridhash,diridcmp)) >= 0) {
	    if ((rs = hdbstr_start(&pip->dirnames,0)) >= 0) {
	        pip->open.dirs = TRUE ;
	    }
	    if (rs < 0)
	        hdb_finish(dbp) ;
	} /* end if (hdb_start) */

	return rs ;
}
/* end subroutine (procdir_begin) */


static int procdir_end(PROGINFO *pip)
{
	HDB		*dbp = &pip->dirs ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.dirs) {
	    pip->open.dirs = FALSE ;

	    if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {
	        DIRID	*dip ;

	        while (hdb_enum(dbp,&cur,&key,&val) >= 0) {
	            dip = (DIRID *) val.buf ;

	            if (dip != NULL) {
	                dirid_finish(dip) ;
	                rs1 = uc_free(dip) ;
	                if (rs >= 0) rs = rs1 ;
	            }

	        } /* end while (enum) */

	        hdb_curend(dbp,&cur) ;
	    } /* end if (cursor) */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_finish(&pip->dirs) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdbstr_finish(&pip->dirnames) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if (was activated) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procdir_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procdir_end) */


static int procdir_have(PROGINFO *pip,dev_t dev,uino_t ino,cchar *np,int nl)
{
	HDB		*dbp = &pip->dirs ;
	HDB_DATUM	key, val ;
	DIRID		did ;
	int		rs ;

	did.ino = ino ;
	did.dev = dev ;

	key.buf = &did ;
	key.len = sizeof(uino_t) + sizeof(dev_t) ;
	if ((rs = hdb_fetch(dbp,key,NULL,&val)) >= 0) {
	    if ((rs = hdbstr_add(&pip->dirnames,np,nl,NULL,0)) >= 0) {
	        rs = 1 ;
	    }
	} else if (rs == SR_NOTFOUND) {
	    if ((rs = procdir_addid(pip,dev,ino)) >= 0) {
	        rs = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (procdir_have) */


static int procdir_addid(PROGINFO *pip,dev_t dev,uino_t ino)
{
	DIRID		*dip ;
	const int	size = sizeof(DIRID) ;
	int		rs ;

	if ((rs = uc_malloc(size,&dip)) >= 0) {
	    if ((rs = dirid_start(dip,dev,ino)) >= 0) {
	        HDB		*dbp = &pip->dirs ;
	        HDB_DATUM	key, val ;
	        key.buf = dip ;
	        key.len = sizeof(uino_t) + sizeof(dev_t) ;
	        val.buf = dip ;
	        val.len = size ;
	        rs = hdb_store(dbp,key,val) ;
	        if (rs < 0)
	            dirid_finish(dip) ;
	    } /* end if (dir-id) */
	    if (rs < 0)
	        uc_free(dip) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (procdir_addid) */


static int procdir_haveprefix(PROGINFO *pip,cchar *fp,int fl)
{
	HDBSTR		*plp = &pip->dirnames ;
	int		rs = SR_OK ;
	int		pl ;
	cchar		*pp ;

	if ((pl = sfdirname(fp,fl,&pp)) > 0) {
	    if ((rs = hdbstr_fetch(plp,pp,pl,NULL,NULL)) >= 0) {
	        rs = 1 ;
	    } else if (rs == SR_NOTFOUND)
	        rs = 0 ;
	}

	return rs ;
}
/* end subroutine (procdir_haveprefix) */


static int procdir_addprefix(PROGINFO *pip,cchar *np,int nl)
{
	HDBSTR		*plp = &pip->dirnames ;
	int		rs ;

	if ((rs = hdbstr_fetch(plp,np,nl,NULL,NULL)) >= 0) {
	    rs = 1 ;
	} else if (rs == SR_NOTFOUND) {
	    if ((rs = hdbstr_add(plp,np,nl,NULL,0)) >= 0) {
	        rs = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (procdir_addprefix) */


static int procuniq_begin(PROGINFO *pip)
{
	HDB		*dbp = &pip->files ;
	const int	n = 50 ;
	const int	at = 1 ;	/* use 'lookaside(3dam)' */
	int		rs ;

	if (pip->f.f_uniq) {
	    if ((rs = hdb_start(dbp,n,at,fileidhash,fileidcmp)) >= 0) {
	        pip->open.files = TRUE ;
	    }
	}

	return rs ;
}
/* end subroutine (procuniq_begin) */


static int procuniq_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.f_uniq && pip->open.files) {
	    HDB		*dbp = &pip->files ;
	    HDB_CUR	cur ;
	    HDB_DATUM	key, val ;
	    FILEID	*dip ;
	    pip->open.files = FALSE ;

	    if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {

	        while (hdb_enum(dbp,&cur,&key,&val) >= 0) {
	            dip = (FILEID *) val.buf ;

	            if (dip != NULL) {
	                fileid_finish(dip) ;
	                rs1 = uc_free(dip) ;
	                if (rs >= 0) rs = rs1 ;
	            }

	        } /* end while (enum) */

	        hdb_curend(dbp,&cur) ;
	    } /* end if (cursor) */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_finish(&pip->files) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if (unique and was activated) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procuniq_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuniq_end) */


static int procuniq_have(PROGINFO *pip,dev_t dev,uino_t ino)
{
	HDB		*dbp = &pip->files ;
	HDB_DATUM	key, val ;
	FILEID		fid ;
	int		rs ;

	fid.ino = ino ;
	fid.dev = dev ;

	key.buf = &fid ;
	key.len = sizeof(uino_t) + sizeof(dev_t) ;
	if ((rs = hdb_fetch(dbp,key,NULL,&val)) >= 0) {
	    rs = 1 ;
	} else if (rs == SR_NOTFOUND) {
	    if ((rs = procuniq_addid(pip,dev,ino)) >= 0) {
	        rs = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (procuniq_have) */


static int procuniq_addid(PROGINFO *pip,dev_t dev,uino_t ino)
{
	FILEID		*dip ;
	const int	size = sizeof(FILEID) ;
	int		rs ;

	if ((rs = uc_malloc(size,&dip)) >= 0) {
	    if ((rs = fileid_start(dip,dev,ino)) >= 0) {
	        HDB		*dbp = &pip->files ;
	        HDB_DATUM	key, val ;
	        key.buf = dip ;
	        key.len = sizeof(uino_t) + sizeof(dev_t) ;
	        val.buf = dip ;
	        val.len = size ;
	        rs = hdb_store(dbp,key,val) ;
	        if (rs < 0)
	            fileid_finish(dip) ;
	    } /* end if (dir-id) */
	    if (rs < 0)
	        uc_free(dip) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (procuniq_addid) */


static int procprune_begin(PROGINFO *pip,cchar *pfname)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = procprune_loadfile(pip,pfname)) >= 0) {
	    if (pip->f.prune) {
	        int	size ;
	        if ((rs = procprune_size(pip,&size)) > 0) {
		    const int	n = rs ;
		    char	*bp ;
		    if ((rs = uc_malloc(size,&bp)) >= 0) {
	    	        PARAMOPT	*pop = &pip->aparams ;
	    	        PARAMOPT_CUR	cur ;
		        pip->prune = (cchar **) bp ;
	    	        if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
			    int		vl ;
	        	    cchar	*po = po_prune ;
	        	    cchar	*vp ;
		            char	**pa = (char **) bp ;
		            bp += ((n+1)*sizeof(void *)) ;
	        	    while (rs >= 0) {
	            	        vl = paramopt_fetch(pop,po,&cur,&vp) ;
	            	        if (vl == SR_NOTFOUND) break ;
	            	        rs = vl ;
	            	        if ((rs >= 0) && (vl > 0)) {
			            pa[c++] = bp ;
			            bp = (strwcpy(bp,vp,vl)+1) ;
	                        }
	                    } /* end while */
		            pa[c] = NULL ;
	                    rs1 = paramopt_curend(pop,&cur) ;
			    if (rs >= 0) rs = rs1 ;
	                } /* end if (paramopt-cur) */
		        if (rs < 0) {
			    uc_free(pip->prune) ;
			    pip->prune = NULL ;
		        }
		    } /* end if (m-a) */
	        } /* end if (procprune_size) */
	    } /* end if (prune) */
	} /* end if (prune-file) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procprune_begin) */


static int procprune_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->prune != NULL) {
	    rs1 = uc_free(pip->prune) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->prune = NULL ;
	}
	return rs ;
}
/* end subroutine (procuniq_end) */


static int procprune_loadfile(PROGINFO *pip,cchar *pfname)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if ((pfname != NULL) && (pfname[0] != '\0')) {
	    bfile	pfile, *pfp = &pfile ;
	    if ((rs = bopen(pfp,pfname,"r",0666)) >= 0) {
		PARAMOPT	*pop = &pip->aparams ;
		const int	llen = LINEBUFLEN ;
		int		len ;
		int		cl ;
		cchar		*po = po_prune ;
		cchar		*cp ;
		char		lbuf[LINEBUFLEN+1] ;
		while ((rs = breadline(pfp,lbuf,llen)) > 0) {
		    len = rs ;
	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;
		    if ((cp = strnchr(lbuf,len,'#')) != NULL) {
			len = (cp-lbuf) ;
		    }
	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    lbuf[((cp-lbuf)+cl)] = '\0' ;
	                    rs = paramopt_loads(pop,po,cp,cl) ;
	                }
	            }
		    if (rs < 0) break ;
		} /* end while */
		rs1 = bclose(pfp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (bfile) */
	    if (c > 0) pip->f.prune = TRUE ;
	} /* end if (have file) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procprune_loadfile) */


static int procprune_size(PROGINFO *pip,int *sizep)
{
	PARAMOPT	*pop = &pip->aparams ;
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	int		size = 0 ;
	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	        int	vl ;
	        cchar	*po = po_prune ;
	        cchar	*vp ;
	        while (rs >= 0) {
	            vl = paramopt_fetch(pop,po,&cur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if ((rs >= 0) && (vl > 0)) {
			c += 1 ;
			size += sizeof(void *) ;
			size += (strnlen(vp,vl)+1) ;
	            }
	        } /* end while */
		size += sizeof(void *) ;
	    rs1 = paramopt_curend(pop,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramopt-cur) */
	if (sizep != NULL) *sizep = size ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procuniq_size) */


static int proclink_begin(PROGINFO *pip)
{
	HDB		*dbp = &pip->links ;
	const int	n = 50 ;
	const int	at = 1 ;	/* use 'lookaside(3dam)' */
	int		rs ;

	rs = hdb_start(dbp,n,at,linkhash,linkcmp) ;
	pip->open.links = (rs >= 0) ;

	return rs ;
}
/* end subroutine (proclink_begin) */


static int proclink_end(PROGINFO *pip)
{
	LINKINFO	*lip ;
	HDB		*dbp = &pip->links ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.links) {
	    pip->open.links = FALSE ;

	    if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {

	        while (hdb_enum(dbp,&cur,&key,&val) >= 0) {
	            lip = (LINKINFO *) val.buf ;

	            if (lip != NULL) {
	                rs1 = linkinfo_finish(lip) ;
	                if (rs >= 0) rs = rs1 ;
	                rs1 = uc_free(lip) ;
	                if (rs >= 0) rs = rs1 ;
	            }

	        } /* end while (enum) */

	        hdb_curend(dbp,&cur) ;
	    } /* end if (cursor) */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_finish(&pip->links) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if (was activated) */

	return rs ;
}
/* end subroutine (proclink_end) */


static int proclink_add(PROGINFO *pip,uino_t ino,cchar *fp)
{
	LINKINFO	*lip ;
	HDB		*dbp = &pip->links ;
	HDB_DATUM	key, val ;
	const int	size = sizeof(LINKINFO) ;
	int		rs ;

	if ((rs = uc_malloc(size,&lip)) >= 0) {
	    if ((rs = linkinfo_start(lip,ino,fp)) >= 0) {
	        key.buf = &lip->ino ;
	        key.len = sizeof(uino_t) ;
	        val.buf = lip ;
	        val.len = size ;
	        rs = hdb_store(dbp,key,val) ;
	        if (rs < 0)
	            linkinfo_finish(lip) ;
	    } /* end if (linkinfo) */
	    if (rs < 0)
	        uc_free(lip) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (proclink_add) */


static int proclink_have(PROGINFO *pip,uino_t ino,LINKINFO **rpp)
{
	HDB		*dbp = &pip->links ;
	HDB_DATUM	key, val ;
	int		rs ;

	key.buf = &ino ;
	key.len = sizeof(uino_t) ;
	if ((rs = hdb_fetch(dbp,key,NULL,&val)) >= 0) {
	    rs = 1 ;
	    if (rpp != NULL) *rpp = (LINKINFO *) val.buf ;
	} else if (rs == SR_NOTFOUND)
	    rs = 0 ;

	return rs ;
}
/* end subroutine (proclink_have) */


static int procsize(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp,
		FILEINFO	*ckp)
{
	ULONG		bytes ;
	ULONG		size ;
	int		rs ;

	if (name == NULL) return SR_FAULT ;

	size = sbp->st_size ;

	bytes = pip->bytes ;
	bytes += (size % MEGABYTE) ;

	pip->bytes = (bytes % MEGABYTE) ;
	pip->megabytes += (bytes / MEGABYTE) ;
	pip->megabytes += (size / MEGABYTE) ;

	rs = (int) (size & INT_MAX) ;
	return rs ;
}
/* end subroutine (procsize) */


static int proclink(pip,name,sbp,ckp)
PROGINFO	*pip ;
cchar		name[] ;
FSDIRTREE_STAT	*sbp ;
FILEINFO	*ckp ;
{
	const mode_t	dm = 0775 ;
	int		rs = SR_OK ;
	int		w = 0 ;
	int		f_dolink = TRUE ;
	char		tarfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: name=%s\n",name) ;
#endif

	if (sbp->st_dev != pip->tarstat.st_dev) {
	    pip->c_linkerr += 1 ;
	    if (! pip->f.nostop) rs = SR_XDEV ;
	    goto ret0 ;
	}

	if ((rs = mkpath2(tarfname,pip->tardname,name)) >= 0) {
	    FSDIRTREE_STAT	tsb ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/proclink: pre tarfname=%s\n",tarfname) ;
#endif

	    if ((rs = fsdirtreestat(tarfname,1,&tsb)) >= 0) {
	        if (! S_ISDIR(sbp->st_mode)) {
	            int	f = TRUE ;
	            f = f && (tsb.st_dev == sbp->st_dev) ;
	            f = f && (tsb.st_ino == sbp->st_ino) ;
	            if (! f) {
	                if (S_ISDIR(tsb.st_mode)) {
	                    w = 1 ;
	                    rs = removes(tarfname) ;
	                } else {
	                    w = 2 ;
	                    rs = uc_unlink(tarfname) ;
	                }
	            } else
	                f_dolink = FALSE ;
	        } else {
	            if (! S_ISDIR(tsb.st_mode)) {
	                w = 3 ;
	                rs = uc_unlink(tarfname) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/proclink: unlink() rs=%d\n",rs) ;
#endif
	            } else
	                f_dolink = FALSE ;
	        }
	    } else if (rs == SR_NOENT)
	        rs = SR_OK ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/proclink: mid rs=%d w=%u f_dolink=%u\n",
	            rs,w,f_dolink) ;
#endif

	    if ((rs >= 0) && f_dolink) {
	        if (! S_ISDIR(sbp->st_mode)) {
	            w = 4 ;
	            rs = uc_link(name,tarfname) ;
	            if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	                w = 5 ;
	                if ((rs = mkpdirs(tarfname,dm)) >= 0) {
	                    w = 6 ;
	                    rs = uc_link(name,tarfname) ;
	                }
	            }
	        } else {
	            w = 7 ;
	            rs = uc_mkdir(tarfname,dm) ;
	            if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	                w = 8 ;
	                if ((rs = mkpdirs(tarfname,dm)) >= 0) {
	                    w = 9 ;
	                    rs = uc_mkdir(tarfname,dm) ;
	                }
	            }
	        }
	    } /* end if (dolink) */

	    if ((rs == SR_EXIST) && (! pip->f.quiet))
	        bprintf(pip->efp,"%s: exists=%u\n",pip->progname,w) ;

	} /* end if (mkpath) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclink: ret rs=%d f_link=%u\n",rs,f_dolink) ;
#endif

	return (rs >= 0) ? f_dolink : rs ;
}
/* end subroutine (proclink) */


static int procsync(pip,name,sbp,ckp)
PROGINFO	*pip ;
cchar		name[] ;
FSDIRTREE_STAT	*sbp ;
FILEINFO	*ckp ;
{
	LINKINFO	*lip ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsync: name=%s\n",name) ;
#endif

/* do we have a link to this file already? */

	if ((rs = proclink_have(pip,sbp->st_ino,&lip)) > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsync: procsynclink()\n") ;
#endif

	    rs = procsynclink(pip,name,sbp,lip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsync: procsynclink() rs=%d\n",rs) ;
#endif

	    if ((rs == SR_NOENT) || (rs == SR_XDEV))
	        rs = procsyncer(pip,name,sbp) ;

	} else if (rs == 0) {

	    rs = procsyncer(pip,name,sbp) ;

	} /* end if (proclink_have) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsync: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsync) */


static int procsynclink(pip,name,sbp,lip)
PROGINFO	*pip ;
cchar		name[] ;
FSDIRTREE_STAT	*sbp ;
LINKINFO	*lip ;
{
	FSDIRTREE_STAT	ssb, dsb ;
	const mode_t	dm = DMODE ;
	int		rs ;
	int		rs1 ;
	int		w = 0 ;
	int		f_dolink = TRUE ;
	char		srcfname[MAXPATHLEN + 1] ;
	char		dstfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: name=%s\n",name) ;
#endif

	rs = mkpath2(srcfname,pip->tardname,lip->fname) ;
	if (rs < 0) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

	rs = fsdirtreestat(srcfname,1,&ssb) ; /* LSTAT */
	if (rs < 0) goto ret0 ;

	rs1 = fsdirtreestat(dstfname,1,&dsb) ; /* LSTAT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: dstfname=%s rs=%d\n",name,rs) ;
#endif

	if (rs1 >= 0) {
	    if (! S_ISDIR(ssb.st_mode)) {
	        int	f = TRUE ;
	        f = f && (ssb.st_dev == dsb.st_dev) ;
	        f = f && (ssb.st_ino == dsb.st_ino) ;
	        if (! f) {
	            if (S_ISDIR(dsb.st_mode)) {
	                w = 1 ;
	                rs = removes(dstfname) ;
	            } else {
	                w = 2 ;
	                rs = uc_unlink(dstfname) ;
	            }
	        } else
	            f_dolink = FALSE ;
	    } else {
	        if (! S_ISDIR(dsb.st_mode)) {
	            w = 3 ;
	            rs = uc_unlink(dstfname) ;
	        } else
	            f_dolink = FALSE ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: mid rs=%d f_dolink=%u\n",
	        rs,f_dolink) ;
#endif

	if ((rs >= 0) && f_dolink) {
	    if (! S_ISDIR(ssb.st_mode)) {
	        w = 4 ;
	        rs = uc_link(srcfname,dstfname) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 5 ;
	            if ((rs = mkpdirs(dstfname,dm)) >= 0) {
	                w = 6 ;
	                rs = uc_link(srcfname,dstfname) ;
	            }
	        }
	    } else {
	        w = 7 ;
	        rs = uc_mkdir(dstfname,dm) ;
	        if ((rs == SR_NOTDIR) || (rs == SR_NOENT)) {
	            w = 8 ;
	            if ((rs = mkpdirs(dstfname,dm)) >= 0) {
	                w = 9 ;
	                rs = uc_mkdir(dstfname,dm) ;
	            }
	        }
	    }
	} /* end if */

	if ((rs == SR_EXIST) && (! pip->f.quiet)) {
	    bprintf(pip->efp,"%s: exists=%u\n",pip->progname,w) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsynclink: ret rs=%d f_link=%u\n",
	        rs,f_dolink) ;
#endif

	return (rs >= 0) ? f_dolink : rs ;
}
/* end subroutine (procsynclink) */


static int procsyncer(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	SIGBLOCK	blocker ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer: name=%s\n",name) ;
#endif

	if ((! S_ISDIR(sbp->st_mode)) && (sbp->st_nlink > 1)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer: proclink_add() ino=%llu\n",
	            sbp->st_ino) ;
#endif

	    rs = proclink_add(pip,sbp->st_ino,name) ;
	}
	if (rs < 0) goto ret0 ;

	if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	    if (S_ISREG(sbp->st_mode)) {

	        rs = procsyncer_reg(pip,name,sbp) ;

	    } else if (S_ISDIR(sbp->st_mode)) {

	        rs = procsyncer_dir(pip,name,sbp) ;

	    } else if (S_ISLNK(sbp->st_mode)) {

	        rs = procsyncer_lnk(pip,name,sbp) ;

	    } else if (S_ISFIFO(sbp->st_mode)) {

	        rs = procsyncer_fifo(pip,name,sbp) ;

	    } else if (S_ISSOCK(sbp->st_mode)) {

	        rs = procsyncer_sock(pip,name,sbp) ;

	    } /* end if */

	    sigblock_finish(&blocker) ;
	} /* end if (blocking signals) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procsyncer) */


static int procsyncer_reg(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	struct utimbuf	ut ;
	struct ustat	dsb ;
	size_t		fsize = 0 ;
	const mode_t	dm = DMODE ;
	const mode_t	nm = (sbp->st_mode & (~ S_IFMT)) | 0600 ;
	uid_t		duid = -1 ;
	int		rs = SR_OK ;
	int		of ;
	int		f_create = FALSE ;
	int		f_update = FALSE ;
	int		f_updated = FALSE ;
	char		dstfname[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procsyncer_reg: tardname=%s\n",pip->tardname) ;
	    debugprintf("main/procsyncer_reg: name=%s\n",name) ;
	}
#endif

	ut.actime = sbp->st_atime ;
	ut.modtime = sbp->st_mtime ;

/* exit on large files! */

	if (sbp->st_size > INT_MAX) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: dstfname=%s\n",dstfname) ;
#endif

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    if (S_ISREG(dsb.st_mode)) {
	        int	f = FALSE ;
	        duid = dsb.st_uid ;
	        fsize = (size_t) dsb.st_size ;
	        f = f || (sbp->st_size != dsb.st_size) ;
	        f = f || (sbp->st_mtime > dsb.st_mtime) ;
	        if (f) {
	            f_update = TRUE ;
	        } /* end if */
	        if (f_update && pip->f.rmfile) {
	            f_create = TRUE ;
	            uc_unlink(dstfname) ;
	        }
	    } else {
	        f_create = TRUE ;
	        if (S_ISDIR(dsb.st_mode)) {
	            rs = removes(dstfname) ;
	        } else
	            rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {

	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        cchar	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procsyncer_reg: dst dname=%t\n",dnp,dnl) ;
#endif
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: no entry source rs=%d\n",
	            rs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    cchar	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        if ((rs = mkpath1w(tmpfname,dnp,dnl)) >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f) {
	                    rs = uc_unlink(tmpfname) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/procsyncer_reg: "
	                            "uc_unlink() rs=%d\n",
	                            rs) ;
#endif
	                }
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,dm) ;
	        } /* end if (mkpath) */
	    }
	} /* end if (need creation) */
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	of = O_WRONLY ;
	if (f_create) of |= O_CREAT ;

	rs = u_open(dstfname,of,nm) ;
	if ((rs == SR_ACCESS) && (! f_create)) {
	    int		dnl ;
	    cchar	*dnp ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: SR_ACCESS\n") ;
#endif
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if ((dnl > 0) && ((rs = mkpath1w(tmpfname,dnp,dnl)) >= 0)) {
	        if ((rs = u_access(tmpfname,W_OK)) >= 0) {
	            f_create = TRUE ;
	            rs = uc_unlink(dstfname) ;
	            of |= O_CREAT ;
	            if (rs >= 0)
	                rs = u_open(dstfname,of,nm) ;
	        }
	    }
	}
	if (rs >= 0) { /* opened */
	    int	dfd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: u_open() rs=%d\n",rs) ;
#endif

	    if (f_create) {
	        rs = uc_fminmod(dfd,nm) ;
	    } /* end if (needed to be created) */

/* do we need to UPDATE the destination? */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_reg: "
	            "need update? rs=%d f_update=%d\n",
	            rs,f_update) ;
#endif

	    if (rs >= 0) {
	        f_updated = TRUE ;
	        if ((rs = u_open(name,O_RDONLY,0666)) >= 0) {
	            int	len ;
	            int	sfd = rs ;

	            rs = uc_copy(sfd,dfd,-1) ;
	            len = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procsyncer_reg: "
	                    "fsize=%u uc_copy() rs=%d\n",
	                    fsize,rs) ;
#endif
	            if ((rs >= 0) && (len != INT_MAX) && (len < fsize)) {
	                offset_t	uoff = len ;
	                rs = uc_ftruncate(dfd,uoff) ;
	            }

	            u_close(sfd) ;
	            if (rs >= 0) {
	                int	f_utime = FALSE ;
	                f_utime = f_utime || (duid < 0) ;
	                f_utime = f_utime || (duid == pip->euid) ;
	                f_utime = f_utime || (pip->euid == 0) ;
	                if (f_utime) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/procsyncer_reg: utime()\n") ;
#endif
	                    u_utime(dstfname,&ut) ;
	                }
	            }
	        } else if (rs == SR_NOENT) {
	            rs = SR_OK ;
	            uc_unlink(dstfname) ;
	        } /* end if (open source) */
	    } /* end if (update was needed) */

	    u_close(dfd) ;
	} /* end if (destination file opened) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_reg: ret rs=%d f_updated=%u\n",
	        rs,f_updated) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_reg) */


static int procsyncer_dir(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	struct utimbuf	ut ;
	struct ustat	dsb ;
	const mode_t	nm = (sbp->st_mode & (~ S_IFMT)) | DMODE ;
	uid_t		duid = -1 ;
	int		rs = SR_OK ;
	int		f_create = FALSE ;
	int		f_update = FALSE ;
	int		f_mode = FALSE ;
	int		f_mtime = FALSE ;
	int		f_updated = FALSE ;
	char		dstfname[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

	ut.actime = sbp->st_atime ;
	ut.modtime = sbp->st_mtime ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    duid = dsb.st_uid ;
	    if (S_ISDIR(dsb.st_mode)) {
	        f_mtime = (dsb.st_mtime != sbp->st_mtime) ;
	        f_mode = (dsb.st_mode != nm) ;
	        if (f_mode || f_mtime) {
	            f_update = TRUE ;
	        }
	    } else {
	        f_create = TRUE ;
	        rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {
	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_dir: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        cchar	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_dir: no entry for source \n") ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_dir: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    cchar	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        rs = mkpath1w(tmpfname,dnp,dnl) ;
	        if (rs >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f)
	                    rs = uc_unlink(tmpfname) ;
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,nm) ;
	        }
	    }
	}
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	if (f_create)
	    rs = mkdir(dstfname,nm) ;

	if ((rs >= 0) && ((duid < 0) || (duid == pip->euid))) {
	    f_updated = TRUE ;
	    if (f_mode) rs = u_chmod(dstfname,nm) ;
	    if ((rs >= 0) && f_mtime) u_utime(dstfname,&ut) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_dir: ret rs=%d f_updated=%u\n",
	        rs,f_updated) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_dir) */


static int procsyncer_lnk(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	struct ustat	dsb ;
	const mode_t	dm = DMODE ;
	int		rs = SR_OK ;
	int		f_create = FALSE ;
	int		f_update = FALSE ;
	int		f_updated = FALSE ;
	char		dstfname[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		dstlink[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procsyncer_lnk: tardname=%s\n",pip->tardname) ;
	    debugprintf("main/procsyncer_lnk: name=%s\n",name) ;
	}
#endif

	rs = u_readlink(name,dstlink,MAXPATHLEN) ;
	if (rs < 0) goto ret0 ;

	rs = mkpath2(dstfname,pip->tardname,name) ;
	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_lstat(dstfname,&dsb)) >= 0) {

	    if (S_ISLNK(dsb.st_mode)) {
	        int	f = TRUE ;
	        f = f && (dsb.st_size == sbp->st_size) ;
	        if (f) {
	            rs = u_readlink(dstfname,tmpfname,MAXPATHLEN) ;
	            if (rs >= 0) f = (strcmp(dstlink,tmpfname) == 0) ;
	        }
	        if (! f) {
	            f_update = TRUE ;
	        } /* end if */
	    } else {
	        f_create = TRUE ;
	        if (S_ISDIR(dsb.st_mode)) {
	            rs = removes(dstfname) ;
	        } else
	            rs = uc_unlink(dstfname) ;
	    }

	} else if (rs < 0) {
	    f_create = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_lnk: dst u_lstat() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTDIR) {
	        int		dnl ;
	        cchar	*dnp ;
	        dnl = sfdirname(dstfname,-1,&dnp) ;
	        rs = SR_OK ;
	        if (dnl > 0) {
	            rs = mkpath1w(tmpfname,dnp,dnl) ;
	            if (rs >= 0)
	                rs = uc_unlink(tmpfname) ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procsyncer_lnk: no entry for source \n") ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_lnk: source rs=%d "
	        "f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* create any needed directories (as necessary) */

	if (f_create) {
	    int		dnl ;
	    cchar	*dnp ;
	    dnl = sfdirname(dstfname,-1,&dnp) ;
	    if (dnl > 0) {
	        rs = mkpath1w(tmpfname,dnp,dnl) ;
	        if (rs >= 0) {
	            struct ustat	sb ;
	            int	rs1 = u_lstat(tmpfname,&sb) ;
	            int	f = FALSE ;
	            if (rs1 >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    rs1 = u_stat(tmpfname,&sb) ;
	                    f = ((rs1 < 0) || (! S_ISDIR(sb.st_mode))) ;
	                } else if (! S_ISDIR(sb.st_mode))
	                    f = TRUE ;
	                if (f)
	                    rs = uc_unlink(tmpfname) ;
	            } else
	                f = TRUE ;
	            if (f)
	                rs = mkdirs(tmpfname,dm) ;
	        }
	    }
	}
	if (rs < 0) goto ret0 ;

/* update (or create) the target file */

	if (rs >= 0) {
	    f_updated = TRUE ;
	    if ((! f_create) && f_update) {
	        rs = uc_unlink(dstfname) ;
	    }
	    if (rs >= 0)
	        rs = u_symlink(dstlink,dstfname) ;
	} /* end if (update was needed) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsyncer_lnk: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procsyncer_lnk) */


static int procsyncer_fifo(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (procsyncer_fifo) */


static int procsyncer_sock(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (procsyncer_sock) */


static int procrm(PROGINFO *pip,cchar name[],FSDIRTREE_STAT *sbp,FILEINFO *ckp)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procrm: name=%s\n",name) ;
#endif

	if (S_ISDIR(sbp->st_mode)) {
	    rs = procrm_add(pip,name,-1) ;
	} else
	    rs = uc_unlink(name) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procrm: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procrm) */


static int dirid_start(DIRID *dip,dev_t dev,uino_t ino)
{
	int		rs = SR_OK ;
	dip->dev = dev ;
	dip->ino = ino ;
	return rs ;
}
/* end subroutine (dirid_start) */


static int dirid_finish(DIRID *dip)
{
	if (dip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (dirid_finish) */


static int fileid_start(FILEID *dip,dev_t dev,uino_t ino)
{
	int		rs = SR_OK ;
	dip->dev = dev ;
	dip->ino = ino ;
	return rs ;
}
/* end subroutine (fileid_start) */


static int fileid_finish(FILEID *dip)
{
	if (dip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (fileid_finish) */


static int linkinfo_start(LINKINFO *lip,uino_t ino,cchar *fp)
{
	int		rs ;
	cchar		*cp ;

	lip->ino = ino ;
	rs = uc_mallocstrw(fp,-1,&cp) ;
	if (rs >= 0) lip->fname = cp ;

	return rs ;
}
/* end subroutine (linkinfo_start) */


static int linkinfo_finish(LINKINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->fname != NULL) {
	    rs1 = uc_free(lip->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (linkinfo_finish) */


/* make *parent* directories as needed */
static int mkpdirs(cchar *tarfname,mode_t dm)
{
	int		rs = SR_OK ;
	int		dl ;
	cchar		*dp ;

	if ((dl = sfdirname(tarfname,-1,&dp)) > 0) {
	    char	dname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath1w(dname,dp,dl)) >= 0) {
	        uc_unlink(dname) ; /* just a little added help */
	        rs = mkdirs(dname,dm) ;
	    }
	} else
	    rs = SR_NOENT ;

	return rs ;
}
/* end subroutine (mkpdirs) */


static int istermrs(int rs)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */


static uint linkhash(const void *vp,int vl)
{
	uint		h = 0 ;
	ushort		*sa = (ushort *) vp ;

	h = h ^ ((sa[1] << 16) | sa[0]) ;
	h = h ^ ((sa[0] << 16) | sa[1]) ;
	if (vl > sizeof(uint)) {
	    h = h ^ ((sa[3] << 16) | sa[2]) ;
	    h = h ^ ((sa[2] << 16) | sa[3]) ;
	}

#if	CF_DEBUGS
	debugprintf("linkhash: h=%08x\n",h) ;
#endif

	return h ;
}
/* end subroutine (linkhash) */


/* ARGSUSED */
static int linkcmp(struct linkinfo *e1p,struct linkinfo *e2p,int len)
{
	int64_t		d ;
	int		rc = 0 ;

	d = (e1p->ino - e2p->ino) ;
	if (d > 0) rc = 1 ;
	else if (d < 0) rc = -1 ;

	return rc ;
}
/* end subroutine (linkcmp) */


static uint diridhash(const void *vp,int vl)
{
	uint		h = 0 ;
	ushort		*sa = (ushort *) vp ;

	h = h ^ ((sa[1] << 16) | sa[0]) ;
	h = h ^ ((sa[0] << 16) | sa[1]) ;
	if (vl > sizeof(uint)) {
	    h = h ^ ((sa[3] << 16) | sa[2]) ;
	    h = h ^ ((sa[2] << 16) | sa[3]) ;
	    if (vl > sizeof(ULONG)) {
	        h = h ^ ((sa[5] << 16) | sa[4]) ;
	        h = h ^ ((sa[4] << 16) | sa[5]) ;
	        if (vl > (4*3)) {
	            h = h ^ ((sa[7] << 16) | sa[6]) ;
	            h = h ^ ((sa[6] << 16) | sa[7]) ;
	        }
	    }
	}

	return h ;
}
/* end subroutine (diridhash) */


static uint fileidhash(const void *vp,int vl)
{
	uint		h = 0 ;
	ushort		*sa = (ushort *) vp ;

	h = h ^ ((sa[1] << 16) | sa[0]) ;
	h = h ^ ((sa[0] << 16) | sa[1]) ;
	if (vl > sizeof(uint)) {
	    h = h ^ ((sa[3] << 16) | sa[2]) ;
	    h = h ^ ((sa[2] << 16) | sa[3]) ;
	    if (vl > sizeof(ULONG)) {
	        h = h ^ ((sa[5] << 16) | sa[4]) ;
	        h = h ^ ((sa[4] << 16) | sa[5]) ;
	        if (vl > (4*3)) {
	            h = h ^ ((sa[7] << 16) | sa[6]) ;
	            h = h ^ ((sa[6] << 16) | sa[7]) ;
	        }
	    }
	}

	return h ;
}
/* end subroutine (fileidhash) */


/* ARGSUSED */
static int diridcmp(struct dirid *e1p,struct dirid *e2p,int len)
{
	int64_t		d ;
	int		rc = 0 ;

	d = (e1p->dev - e2p->dev) ;
	if (d == 0) d = (e1p->ino - e2p->ino) ;

	if (d > 0) rc = 1 ;
	else if (d < 0) rc = -1 ;

	return rc ;
}
/* end subroutine (diridcmp) */


/* ARGSUSED */
static int fileidcmp(struct fileid *e1p,struct fileid *e2p,int len)
{
	int64_t		d ;
	int		rc = 0 ;

	d = (e1p->dev - e2p->dev) ;
	if (d == 0) d = (e1p->ino - e2p->ino) ;

	if (d > 0) rc = 1 ;
	else if (d < 0) rc = -1 ;

	return rc ;
}
/* end subroutine (fileidcmp) */


static int vcmprstr(cchar **e1pp,cchar **e2pp)
{
	if ((*e1pp == NULL) && (*e2pp == NULL)) return 0 ;
	if (*e1pp == NULL) return 1 ;
	if (*e2pp == NULL) return -1 ;
	return (- strcmp(*e1pp,*e2pp)) ;
}
/* end subroutine (vcmprstr) */


static int isAccess(int rs)
{
	return isOneOf(accrs,rs) ;
}
/* end subroutine (isAccess) */


