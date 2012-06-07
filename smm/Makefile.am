# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS} ${libHX_CFLAGS}
AM_CFLAGS   = ${regular_CFLAGS}
CLEANFILES  = omixer orec

bin_PROGRAMS = \
	oplay

dist_bin_SCRIPTS = \
	googtts gpsh ssa2srt

pkglibexec_PROGRAMS = \
	bsvplay \
	pcmdiff \
	qplay \
	vfontas

dist_pkglibexec_SCRIPTS  = \
	extract_d3pkg \
	extract_dxhog \
	extract_f3pod \
	extract_qupak \
	fnt2bdf \
	mod2ogg \
	mpg2ogg \
	png2wx.pl \
	png2wx.py \
	shared.pm \
	stxdb

bsvplay_SOURCES = bsvplay.c pcspkr_pcm.c
bsvplay_LDADD   = ${libHX_LIBS} -lm
oplay_LDADD     = ${libHX_LIBS}
pcmdiff_LDADD   = ${libHX_LIBS}
qplay_SOURCES   = qplay.c pcspkr_pcm.c
qplay_LDADD     = ${libHX_LIBS} -lm
vfontas_LDADD   = ${libHX_LIBS}

EXTRA_DIST = pcspkr.h

omixer${EXEEXT}: oplay${EXEEXT}
	if [ ! -e omixer ]; then ln -s oplay omixer; fi;

orec${EXEEXT}: oplay${EXEEXT}
	if [ ! -e orec ]; then ln -s oplay orec; fi;

all-local: omixer${EXEEXT} orec${EXEEXT}

install-exec-local:
	test -z "${bindir}" || ${MKDIR_P} "${DESTDIR}${bindir}";
	ln -s "oplay${EXEEXT}" "${DESTDIR}${bindir}/orec${EXEEXT}";
	ln -s "oplay${EXEEXT}" "${DESTDIR}${bindir}/omixer${EXEEXT}";
