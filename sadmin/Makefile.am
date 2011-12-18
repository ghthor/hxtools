# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS}
AM_CFLAGS   = ${regular_CFLAGS} ${libHX_CFLAGS}

bin_PROGRAMS = \
	newns \
	ofl

dist_bin_SCRIPTS = \
	pmap_dirty \
	su1

pkglibexec_PROGRAMS = \
	fd0ssh \
	printcaps \
	proc_iomem_count \
	psthreads \
	utmp_register

dist_pkglibexec_SCRIPTS = \
	logontime \
	netload \
	recursive_lower \
	vcsaview

noinst_PROGRAMS = \
	xfs_irecover

dist_noinst_SCRIPTS = \
	filenameconv \
	raregetty \
	rpmdep.pl

ofl_LDADD              = ${libHX_LIBS}
printcaps_LDADD        = ${libcap_LIBS}
proc_iomem_count_LDADD = ${libHX_LIBS}
psthreads_LDADD        = ${libHX_LIBS}
utmp_register_LDADD    = ${libHX_LIBS}
xfs_irecover_LDADD     = ${libHX_LIBS}
