# -*- Makefile -*-

x11fonts     = mux.pcf.gz rhx.pcf.gz tosh.pcf.gz
x11miscdir   = ${x11fontdir}/misc
x11misc_DATA = ${x11fonts}
CLEANFILES   = ${x11fonts}

qwertydir         = ${kbddatadir}/keymaps/i386/qwerty
dist_qwerty_DATA  = us_jng.map us_jng_vaiou3.map

qwertzdir         = ${kbddatadir}/keymaps/i386/qwertz
dist_qwertz_DATA  = de_jng.map

vgafontdir        = ${kbddatadir}/consolefonts
dist_vgafont_DATA = A1.fnt B1.fnt ati.fnt gotik-textura.fnt ital.fnt mux.fnt \
               neuropol.fnt nvidia.fnt \
               rhx.fnt tosh.fnt \
               ega437_8x16.fnt ega437_8x14.fnt ega437_8x8.fnt

unimapdir        = ${kbddatadir}/unimaps
dist_unimap_DATA = cp437AB.uni

EXTRA_DIST = ahnv.bdf mux.bdf rhx.bdf tosh.bdf
BDFTOPCF = ${AM_V_GEN} bdftopcf <"$<" | gzip -9c >"$@"

ahnv.pcf.gz: ahnv.bdf
	${BDFTOPCF}

mux.pcf.gz: mux.bdf
	${BDFTOPCF}

rhx.pcf.gz: rhx.bdf
	${BDFTOPCF}

tosh.pcf.gz: tosh.bdf
	${BDFTOPCF}
