/*  fetchfrom */


#define	CF_DEBUG	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *		B.R.Schatz						*
 *									*
 ************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* find "FROM" field when message has been generated by UNIX mail
	   i.e. look for "From" or ">From" (for remote mail). */

fetchfrom (fp,str,len)
FILE *fp ;
int len ;
char str[] ;
{


	char temp[BUFSIZE],last[BUFSIZE],sender[50], *word, *strtok() ;


	rewind(fp) ;

/* find last From line (original sender) */

	while (fgetline(fp,temp,BUFSIZE) > 0) {

	    if ((strncmp(temp,"From",4) != 0)  &&
	        (strncmp(temp,">From",5) != 0)) break ;

	    strcpy (last,temp) ;

	}
#ifdef DEBUG
	fprintf(errlog, "fetchfrom: the last from line - %s\n",last) ;
#endif

	strtok(last," ");	/* the "From" */

	strncpy(str, strtok(0," "),len);/* the sender login name */

#ifdef DEBUG
	fprintf(errlog, "fetchfrom: sender's login name - %s\n",str) ;
#endif

/* find remote machine name if there is one */

	while ((word = strtok(0," \t"))  !=  NULL) {

	    if (strcmp(word,"from") == 0) {

/* found "remote from" */

	        strcpy (sender,str) ;
	        strcpy (str, strtok(0," \n")) ; /* machine */
#ifdef DEBUG
	        fprintf(errlog, "fetchfrom: sender's machine name - %s\n",str) ;
#endif
	        strcat (str, "!") ;

	        strcat (str, sender) ;

	        break ;
	    }
	}
}

