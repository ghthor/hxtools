# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS} ${libHX_CFLAGS}
AM_CFLAGS   = ${regular_CFLAGS}

bin_PROGRAMS = \
	declone \
	tailhex \
	xcp

dist_bin_SCRIPTS = \
	fduphl \
	fxterm \
	pesubst \
	qtar \
	sysinfo \
	wktimer

dist_pkglibexec_SCRIPTS = \
	mailsplit

tailhex_LDADD = ${libHX_LIBS}
xcp_LDADD     = ${libHX_LIBS}
