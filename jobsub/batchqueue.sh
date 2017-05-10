#!/usr/bin/sh
# batchqueue - list jobs in batch queues
#  Options:
#       -q queuename
#               Examine only the named queue (default is all queues).
#       -l
#               give long listing with status lines
#       -a [user ...]
#               Report jobs owned by the invoking user only,
#               and the list of users given (if any).

# Need /bsd43/bin in front of PATH for MIPS RISCos (so we get BSD `ls').
PATH="/usr/add-on/local/bin:/bin:/sbin:/usr/bin:/usr/sbin"; export PATH

LSOPT=-l
cd SPOOLDIR

if [ -f .queueorder ]; then
	dirs=`cat .queueorder`
	for dirname in *
	do
		grep -s $dirname .queueorder >/dev/null
		if [ "$?" != "0" ]; then
			echo "$dirname not in .queueorder - see your system administrator"
			dirs="$dirs $dirname"
		fi
	done
else
	dirs="*"
fi

case "$1" in
-q)
        shift
        case $# in
        0) echo "${0}: -q: Queue list missing; default used." 1>&2 ;;
        *) dirs="$1"; shift;;
        esac
        ;;
esac

case "$1" in
-l)     LONG=1;shift;;
esac

case "$1" in
-a*) who=${USER-`whoami`}; shift;;
esac

case $# in
0)
        case "$who" in
        "") who=".*" ;;
        esac
        ;;



*)
        case "$who" in
        "") who="$1";shift;;
        esac
        for x do
                who="$who|$1"
                shift
        done
        ;;
esac

for dir in $dirs ; do
        if [ ! -d "$dir" ]; then
                continue
        fi
        cd $dir
        echo ">>-- $dir queue --<<"
        if [ $LONG ]; then
                if [ -f queuestat ]; then
                        cat queuestat
                fi
        fi
	cd CFDIR
	# OSF/1: change $8 on next line to $9
        for file in `ls $LSOPT cf* 2>/dev/null | awk "/ ($who) /"' { print $8
}'`
        do
		trailer=`echo $file | sed -e 's=^cf=='`
                ls $LSOPT $file | \
                # OSF/1: change next line to:
		# awk '{ printf("%-12s %s %2s %5s   %s", $3, $6, $7, $8, substr($9,3) ) }'
		awk '{ printf("%-12s %s %2s %5s   %s", $3, $5, $6, $7, substr($8,3) ) }'
		if [ -f ../of$trailer ]; then
			echo "   *** Executing ***"
		else
			echo "   *** Queued ***"
		fi
                grep "Job cancelled" $file 2>/dev/null || \
                sed     -e "1,/# BATCH: Start of User Input/d" \
                        -e "/^$/d" \
                        -e "s/'//g;s/  */ /g;s/^ */  /;" \
                        -e "/typeset/s;/tmp/typesetSTDIN_;#;" \
                        -e "/spice/s;/tmp/spiceSTDIN_;#;" \
                        2>/dev/null $file
        done
        cd ../..
done
exit 0