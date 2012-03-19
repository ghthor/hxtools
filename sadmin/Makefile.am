# -*- Makefile -*-

AM_CPPFLAGS = ${regular_CPPFLAGS} ${libHX_CFLAGS}
AM_CFLAGS   = ${regular_CFLAGS}

bin_PROGRAMS = \
	clock_info \
	newns \
	ofl

dist_bin_SCRIPTS = \
	pmap_dirty \
	su1

pkglibexec_PROGRAMS = \
	fd0ssh \
	printcaps \
	proc_iomem_count \
	proc_stat_parse \
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

clock_info_LDADD       = -lrt
ofl_LDADD              = ${libHX_LIBS}
printcaps_LDADD        = ${libHX_LIBS} ${libcap_LIBS}
proc_iomem_count_LDADD = ${libHX_LIBS}
proc_stat_parse_LDADD  = ${libHX_LIBS}
psthreads_LDADD        = ${libHX_LIBS}
utmp_register_LDADD    = ${libHX_LIBS}
xfs_irecover_LDADD     = ${libHX_LIBS}
