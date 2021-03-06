 # <-- force CSH to use Bourne shell
# MSGSD


DOMAIN=`logname - domain`

if [ -d /usr/sbin ] ; then
  MACH=`uname -n`
else
  MACH=`hostname`
fi


case ${MACH}.${DOMAIN} in

mt* )
  SUBGROUP=mt
  : ${LOCAL:=/mt/mtgzfs8/hw/add-on/local}
  ;;

rc*.rightcore.com )
  SUBGROUP=rc
  : ${NAME:="MSGSD - messages daemon"}
  export NAME
  ;;

esac


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
export PCS LOCAL TOOLS


if [ -n "${SUBGROUP}" ] ; then
  : ${MSGS_SPOOLDIR:=${PCS}/spool/boards/msgs/${SUBGROUP}}
  export MSGS_SPOOLDIR
fi

VERSION="0a"


PATH=${PCS}/bin:${PATH}
export PATH


U=$LOGNAME
if [ -n "${1}" ] ; then
  U=$1
fi

N=""
if [ -n "${2}" ] ; then
  N=`basename ${2} `
fi

pcsconf -l msgsd -p msgsd-${VERSION} -y "u=${U} j=${N}"

NAME="PCS MSGSD"
export NAME

exec msgs -s



