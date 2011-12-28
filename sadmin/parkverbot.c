/*
 *	parkverbot - inhibit hard disk head parking
 *	Copyright © Jan Engelhardt, 2011
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of the
 *	License, or (at your option) any later version.
 */
#define _FILE_OFFSET_BITS 64
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mount.h>
#include <libHX/init.h>
#include <libHX/list.h>
#include <libHX/misc.h>
#include <libHX/option.h>

/**
 * @anchor:	anchor for linkage with @pv_bdev_lsit
 * @path:	device path, for informational purposes
 * @size:	size of the device
 * @fd:		file descriptor of the opened device
 */
struct pv_bdev_entry {
	struct HXlist_head anchor;
	const char *path;
	loff_t size;
	int fd;
};

static HXLIST_HEAD(pv_bdev_list);
static struct timespec pv_req_interval = {4, 0};
static unsigned long long pv_disk_window = 16384;

static void pv_mainloop(void)
{
	const struct pv_bdev_entry *e;
#define bsiz 64*1024
	char buf[bsiz];
	unsigned long long new_pos;
	ssize_t read_ret;

	while (true) {
	HXlist_for_each_entry(e, &pv_bdev_list, anchor) {
		new_pos = HX_drand(0, e->size);
		printf("%s: %llu\n", e->path, new_pos);
		if (lseek(e->fd, new_pos, SEEK_SET) < 0)
			fprintf(stderr, "%s: lseek: %s\n",
			        e->path, strerror(errno));

		read_ret = read(e->fd, buf, bsiz);
		if (read_ret < 0)
			fprintf(stderr, "%s: read: %s\n",
			        e->path, strerror(errno));

		nanosleep(&pv_req_interval, NULL);
	}
	}
}

static bool pv_open_device(const char *path)
{
	struct pv_bdev_entry *e;
	size_t size;
	int fd;

	fd = open(path, O_RDONLY | O_BINARY);
	if (fd < 0) {
		fprintf(stderr, "%s: %s\n", path, strerror(errno));
		return false;
	}
	if (ioctl(fd, BLKGETSIZE64, &size) < 0) {
		fprintf(stderr, "%s: BLKGETSIZE64: %s\n", path, strerror(errno));
		return false;
	}
	e = malloc(sizeof(*e));
	if (e == NULL) {
		fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
		close(fd);
		return false;
	}
	HXlist_init(&e->anchor);
	e->path = path;
	e->size = size;
	e->fd   = fd;
	HXlist_add_tail(&pv_bdev_list, &e->anchor);
	return true;
}

static bool pv_get_options(int *argc, const char ***argv)
{
	const char **arg;
	double intv = 0;
	struct HXoption options_table[] = {
		{.sh = 'r', .type = HXTYPE_ULLONG, .ptr = &pv_disk_window,
		 .help = "Guard window size, in KB (default: 16384)",
		 .htyp = "kbytes"},
		{.sh = 't', .type = HXTYPE_DOUBLE, .ptr = &intv,
		 .help = "Interval between requests, in s (default: 4.0)",
		 .htyp = "sec"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) !=
	    HXOPT_ERR_SUCCESS)
		return false;

	if (intv != 0) {
		pv_req_interval.tv_sec  = intv;
		pv_req_interval.tv_nsec = modf(intv, &intv);
	}
	pv_disk_window <<= 10;

	if (*argc < 2) {
		fprintf(stderr, "You need to specify some devices to "
		        "keep spinning.\n");
		return false;
	}
	for (arg = &(*argv)[1]; *arg != NULL; ++arg)
		pv_open_device(*arg);
	return true;
}

int main(int argc, const char **argv)
{
	int ret;

	if ((ret = HX_init()) < 0) {
		fprintf(stderr, "HX_init: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}

	if (!pv_get_options(&argc, &argv))
		return EXIT_FAILURE;

	pv_mainloop();
	return EXIT_SUCCESS;
}
