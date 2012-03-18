# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS} ${libHX_CFLAGS} ${libmount_CFLAGS} \
              ${libpci_CFLAGS} ${libxcb_CFLAGS}
AM_CFLAGS   = ${regular_CFLAGS}

bin_PROGRAMS = \
	declone \
	sysinfo \
	tailhex \
	xcp

dist_bin_SCRIPTS = \
	fduphl \
	fxterm \
	pesubst \
	qtar \
	wktimer

dist_pkglibexec_SCRIPTS = \
	mailsplit

sysinfo_LDADD = ${libHX_LIBS} ${libmount_LIBS} ${libpci_LIBS} ${libxcb_LIBS}
tailhex_LDADD = ${libHX_LIBS}
xcp_LDADD     = ${libHX_LIBS}
