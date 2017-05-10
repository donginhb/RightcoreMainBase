# MAKEFILE

T= bibleq

BQST= bibleqs

TIST= txtindexes

MKST= txtindexmks

ALL= $(T) $(BQST).so $(TIST).so $(MKST).so

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
EXTRALIB= $(EXTRA)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O3 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

GLD= gcc

LDRPATH= $(EXTRA)/lib

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH) -m
# profiling; using 'profil(1)'
LDFLAGS_P= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)
# profiling; using 'gprof(1)'
LDFLAGS_GP= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)
# profiling GCC; using 'gprof(1)'
LDFLAGS_GGP= $(LF_LDFLAGS) -pg


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


GGLIBDIRS= -L$(LIBDIR)

GGLIB0=
GGLIB1= -ldam -lb
GGLIB2=
GGLIB3= -luc -lu
GGLIB4= 
GGLIB5= 
GGLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
GGLIB7= -ldl

GGLIBA= $(GGLIB0) $(GGLIB1) $(GGLIB2) $(GGLIB3) 
GGLIBB= $(GGLIB4) $(GGLIB5) $(GGLIB6) $(GGLIB7)

GGLIBS= $(GGLIBA) $(GGLIBB)


SLIBDIRS= -L$(LIBDIR)

SLIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6)


CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= b_$(T).h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= b_$(T).o
OBJ02= bibleq.o biblebook.o 
OBJ03= 
OBJ04= 
OBJ05= ndigits.o sfword.o 
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o

OBJ08=
OBJ09= field_wordphrase.o
OBJ10= wordfill.o
OBJ11= biblepara.o
OBJ12= eigendb.o xwords.o txtindex.o txtindexmk.o 
OBJ13= dirseen.o modload.o
OBJ14= 
OBJ15= 

#OBJ16= bibleqs.o searchkeys.o
#OBJ17= txtindexmks.o txtindexes.o 
#OBJ18= bibleparas.o bpimk.o bpi.o bpihdr.o bvcitekey.o
#OBJ19= offindex.o strtab.o
#OBJ20= txtindexfu.o strtabfind.o
#OBJ21=
#OBJ22=
#OBJ23=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)

GGOBJS= $(OBJ)

BQSOBJ0= bibleqs.o xwords.o txtindex.o txtindexmk.o eigendb.o
BQSOBJ1= sfword.o searchkeys.o modload.o

BQSOBJ= $(BQSOBJ0) $(BQSOBJ1)

TISOBJ= txtindexes.o txtindexfu.o offindex.o strtabfind.o

MKSOBJ= txtindexmks.o txtindexfu.o strtab.o strtabfind.o


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_GP) $(GOBJS) $(LIBDIRS) $(LIBS) 

$(T).ggprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(GLD) -o $@ $(LDFLAGS_GGP) $(GGOBJS) $(GGLIBDIRS) $(GGLIBS)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

so:			$(TIST).so $(MKST).so $(BQST).so

$(BQST).so:		$(BQSOBJ) Makefile $(BQST).map
	$(LD) -G -o $@ -M $(BQST).map $(LDFLAGS) \
		$(BQSOBJ) $(SLIBDIRS) $(SLIBS) > $(BQST).lm

$(TIST).so:		$(TISOBJ) Makefile $(TIST).map
	$(LD) -G -o $@ -M $(TIST).map $(LDFLAGS) \
		$(TISOBJ) $(SLIBDIRS) $(SLIBS) > $(TIST).lm

$(MKST).so:		$(MKSOBJ) Makefile $(MKST).map
	$(LD) -G -o $@ -M $(MKST).map $(LDFLAGS) \
		$(MKSOBJ) $(SLIBDIRS) $(SLIBS) > $(MKST).lm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(TISOBJ) $(MKSOBJ) $(BQSOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install-so:		$(TIST).so $(MKST).so $(BQST).so
	makenewer -r $(TIST).so $(MKST).so $(BQST).so $(LIBDIR)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm
	rm -f $(TIST).so $(MKST).so $(BQST).so

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) maininfo.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

b_$(T).o:		b_$(T).c $(INCS) bibleq.h

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h


sfword.o:		sfword.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexmkwait.o:	txtindexmkwait.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr_adds.o:		vecstr_adds.c

vecstr_adduniq.o:	vecstr_adduniq.c

vecstr_adduniqs.o:	vecstr_adduniqs.c

vecstr.o:		vecstr.c vecstr.h

keyopt.o:		keyopt.c keyopt.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

biblebook.o:		biblebook.c biblebook.h

bibleq.o:		bibleq.c bibleq.h bibleqs.h

wordfill.o:		wordfill.c wordfill.h

strstore.o:		strstore.c strstore.h

biblepara.o:		biblepara.c biblepara.h bibleparas.h


biblebooks.o:		biblebooks.c biblebooks.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

bibleqs.o:		bibleqs.c bibleqs.h txtindex.h txtindexmk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindex.o:		txtindex.c txtindex.h txtindexes.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexmk.o:		txtindexmk.c txtindexmk.h txtindexmks.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexes.o:		txtindexes.c txtindexes.h txtindexfu.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexmks.o:		txtindexmks.c txtindexmks.h txtindexfu.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexfu.o:		txtindexfu.c txtindexfu.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

offindex.o:		offindex.c offindex.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

modload.o:		modload.c modload.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

xwords.o:		xwords.c xwords.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

eigendb.o:		eigendb.c eigendb.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

searchkeys.o:		searchkeys.c searchkeys.h xwords.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

strtab.o:		strtab.c strtab.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

bibleparas.o:		bibleparas.c bibleparas.h bpi.h bpimk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

bpi.o:			bpi.c bpi.h bpihdr.h bvcitekey.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

bpimk.o:		bpimk.c bpimk.h bpihdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

bpihdr.o:		bpihdr.c bpihdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vecint.o:		vecint.c vecint.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

setint.o:		setint.cc setint.h
	$(GPP) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) setint.cc

ts.o:			ts.c ts.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

