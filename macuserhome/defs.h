/* defs */

/* Copyright � 1998 David A�D� Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<dater.h>
#include	<ids.h>
#include	<localmisc.h>


#ifndef	nelem
#ifdef	nelements
#define	nelem		nelements
#else
#define	nelem(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif

#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#ifndef	LOGNAME_MAX
#define	GROUPNAMELEN	LOGNAME_MAX
#else
#define	GROUPNAMELEN	32
#endif
#endif

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PROJNAMELEN
#ifndef	LOGNAME_MAX
#define	PROJNAMELEN	LOGNAME_MAX
#else
#define	PROJNAMELEN	32
#endif
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

/* timezone (zoneinfo) name */
#ifndef	TZLEN
#define	TZLEN		60
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	14
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	NFILE
#define	NFILE		20
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	BUFLEN		MAX((MAXPATHLEN + MAXNAMELEN),MAXHOSTNAMELEN)

#ifndef	EX_OLDER
#define	EX_OLDER	1
#endif

#ifndef	EX_UNKNOWN
#define	EX_UNKNOWN	2
#endif


struct proginfo_flags {
	uint		outfile :1;
	uint		errfile :1;
	uint		kopts :1;
	uint		aparams :1;
	uint		configfile :1;
	uint		pidfile :1;
	uint		lockfile :1;
	uint		logfile :1;
	uint		msfile :1;
	uint		quiet :1;
	uint		config :1;
	uint		pidlock :1;
	uint		daemon :1;
	uint		log :1;
	uint		speed :1;
	uint		geekout :1;
	uint		nodeonly :1;
	uint		tmpdate :1;
	uint		disable :1;
	uint		all :1;
	uint		def :1;
	uint		list :1;
	uint		linebuf :1;
	uint		runint :1;
	uint		disint :1;
	uint		pollint :1;
	uint		lockint :1;
	uint		markint :1;
	uint		touch_toucht :1;
	uint		touch_access :1;
	uint		touch_create :1;
	uint		touch_modify :1;
	uint		chacl_delempty :1;
	uint		chacl_recurse :1;
	uint		chacl_nostop :1;
	uint		chacl_follow :1;
	uint		chacl_min :1;
	uint		chacl_max :1;
	uint		chacl_maskcalc :1;
	uint		msu_msfile :1;
	uint		msu_pollint :1;
	uint		msu_runint :1;
	uint		msu_zerospeed :1;
	uint		msinfo_age :1;
	uint		msinfo_zerospeed :1;
	uint		msinfo_zeroentry :1;
	uint		rest_poll :1;
	uint		mailforward_maildir :1;
	uint		mailforward_pollint :1;
	uint		mailforward_runint :1;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progename ;		/* execution filename */
	char		*progdname ;		/* dirname of arg[0] */
	char		*progname ;		/* basename of arg[0] */
	char		*pr ;			/* program root */
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*nodename ;
	char		*rootname ;		/* distribution name */
	char		*domainname ;
	char		*username ;
	char		*groupname ;
	char		*tmpdname ;
	char		*maildname ;
	char		*helpfname ;
	void		*efp ;
	void		*buffer ;
	void		*cip ;			/* configuration information */
	void		*config ;		/* for MSU */
	void		*contextp ;
	struct proginfo_flags	f ;
	struct proginfo_flags	have ;
	struct proginfo_flags	final ;
	struct proginfo_flags	open ;
	struct proginfo_flags	change ;
	struct timeb	now ;
	LOGFILE		lh ;
	DATER		tmpdate, mdate ;
	IDS		id ;
	time_t		daytime ;
	time_t		lastcheck ;
	pid_t		pid ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		n ;
	int		loglen ;
	int		runint ;
	int		disint ;		/* disable interval */
	int		pollint ;		/* poll interval */
	int		markint ;
	int		lockint ;
	int		speedint ;
	char		logid[LOGIDLEN + 1] ;
	char		cmd[LOGIDLEN + 1] ;
	char		logfname[MAXNAMELEN + 1] ;
	char		msfname[MAXNAMELEN + 1] ;
	char		pidfname[MAXNAMELEN + 1] ;
	char		speedname[MAXNAMELEN + 1] ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,const char *,
		const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,const char *,int,const char **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


