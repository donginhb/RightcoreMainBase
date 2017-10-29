/* config */


/* revision history:

	= 2000-05-14, David A�D� Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright � 2000 David A�D� Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)homepage "
#define	BANNER		"HomePage"
#define	SEARCHNAME	"homepage"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	VARPROGRAMROOT1	"HOMEPAGE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"HOMEPAGE_BANNER"
#define	VARSEARCHNAME	"HOMEPAGE_NAME"
#define	VAROPTS		"HOMEPAGE_OPTS"
#define	VARFILEROOT	"HOMEPAGE_FILEROOT"
#define	VARLOGTAB	"HOMEPAGE_LOGTAB"
#define	VARBASENAME	"HOMEPAGE_VARBASE"
#define	VARQS		"HOMEPAGE_QS"
#define	VARQUERYSTRING	"HOMEPAGE_QUERYSTRING"
#define	VARCFNAME	"HOMEPAGE_CF"
#define	VARAFNAME	"HOMEPAGE_AF"
#define	VARWFNAME	"HOMEPAGE_WF"
#define	VAREFNAME	"HOMEPAGE_EF"
#define	VARDEBUGFNAME	"HOMEPAGE_DEBUGFILE"
#define	VARDEBUGFD1	"HOMEPAGE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	WORKDNAME	"/tmp"
#define	ETCCNAME	"etc"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"
#define	LWFNAME		"/etc/logwelcome"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	FULLFNAME	".fullname"
#define	HEADFNAME	"head.txt"
#define	SVCFNAME	"svc"

#define	ETCDNAME	"etc/homepage"
#define	PIDFNAME	"run/homepage"		/* mutex PID file */
#define	LOGFNAME	"var/log/homepage"	/* activity log */
#define	LOCKFNAME	"spool/locks/homepage"	/* lock mutex file */

#define	COLUMNS		80

#define	LOGSIZE		(80*1024)

#define	DEFTERMTYPE	"text"
#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_CACHE	(5*60)			/* cache time-out (seconds) */

#define	OPT_LOGPROG	1			/* (program) logging */
#define	OPT_CACHE	1			/* use cache */


