# -*- Makefile -*-

x11fonts     = ahnv.pcf.gz mux.pcf.gz rhx.pcf.gz tosh.pcf.gz
x11miscdir   = ${x11fontdir}/misc
x11misc_DATA = ${x11fonts}
CLEANFILES   = ${x11fonts}

qwertydir    = ${keymapdir}/i386/qwerty
dist_qwerty_DATA  = us_jng.map us_jng_vaiou3.map

qwertzdir    = ${keymapdir}/i386/qwertz
dist_qwertz_DATA  = de_jng.map

dist_vgafont_DATA = ahnv.fnt ati.fnt gotik-textura.fnt ital.fnt mux.fnt \
               neuropol.fnt nvidia.fnt \
               rhx.fnt tosh.fnt \
               ega437_8x16.fnt ega437_8x14.fnt ega437_8x8.fnt

EXTRA_DIST = ahnv.bdf mux.bdf rhx.bdf tosh.bdf
BDFTOPCF = bdftopcf <"$<" | gzip -9c >"$@"

ahnv.pcf.gz: ahnv.bdf
	${BDFTOPCF}

mux.pcf.gz: mux.bdf
	${BDFTOPCF}

rhx.pcf.gz: rhx.bdf
	${BDFTOPCF}

tosh.pcf.gz: tosh.bdf
	${BDFTOPCF}
