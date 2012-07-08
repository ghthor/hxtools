/*
 *	sysinfo.c -
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2012
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <libHX/ctype_helper.h>
#include <libHX/defs.h>
#include <libHX/io.h>
#include <libHX/map.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <pci/header.h>
#include <pci/pci.h>
#include <xcb/xcb.h>
#include <libmount.h>

struct sy_block {
	struct utsname uts;
	unsigned int num_cpu_threads, num_tasks;
	char *cpu_vendor, *cpu_arch;
	hxmc_t *cpu_model_name;
	double cpu_mhz, load_avg1;
	unsigned long long mem_used, mem_total;
	unsigned long long disk_used, disk_total;
	char gfx_hardware_buf[128];
	const char *gfx_hardware;
	unsigned int display_width, display_height;
};

static const char *sy_cpuinfo_file = "/proc/cpuinfo";
static const char sysfs_cpu_dir[] = "/sys/devices/system/cpu";

static void sy_num_cpu_threads(struct sy_block *sib)
{
	struct HXdir *dh = HXdir_open(sysfs_cpu_dir);
	unsigned int thr = 0;
	const char *de;
	struct stat sb;
	char path[80];

	if (dh == NULL)
		return;
	while ((de = HXdir_read(dh)) != NULL) {
		if (strncmp(de, "cpu", 3) != 0)
			continue;
		snprintf(path, sizeof(path), "%s/%s/topology",
		         sysfs_cpu_dir, de);
		if (stat(path, &sb) == 0)
			++thr;
	}
	HXdir_close(dh);
	sib->num_cpu_threads = thr;
}

static char *__HX_strmtrim(char *i)
{
	char *orig, *out, last = '\0';

	for (orig = out = i; *i != '\0'; ++i) {
		if (HX_isspace(last) && *i == last)
			continue;
		*out++ = last = *i;
	}
	*out++ = '\0';
	return orig;
}

static hxmc_t *sy_cpu_model(const char *name)
{
	hxmc_t *buf;
	char *p;

	if (strncmp(name, "Dual-Core", 9) == 0 ||
	    strncmp(name, "Quad-Core", 9) == 0)
		name += 9;
	while (HX_isspace(*name))
		++name;
	buf = HXmc_strinit(name);
	if (buf == NULL)
		return NULL;

	/* Kill ", altivec supported" and "@ 2.67 GHz" */
	p = strpbrk(buf, "@,");
	if (p != NULL)
		*p = '\0';

	HX_strrtrim(buf);
	__HX_strmtrim(buf);
	return buf;
}

static char *sy_hop_colon(char *key, char *value)
{
	while (*value != ':' && *value != '\0')
		++value;
	if (*value != ':')
		return NULL;
	*value++ = '\0';
	HX_strrtrim(key);
	while (HX_isspace(*value))
		++value;
	return value;
}

static void sy_proc_cpuinfo(struct sy_block *sib)
{
	FILE *fp = fopen(sy_cpuinfo_file, "r");
	hxmc_t *line = NULL;
	char *key, *value;
	unsigned int dummy_uint;

	if (fp == NULL)
		return;
	while (HX_getl(&line, fp) != NULL) {
		value = key = line;
		value = sy_hop_colon(key, value);
		if (value == NULL)
			continue;
		HX_chomp(value);

		if (strcmp(key, "cpu") == 0 || strcmp(key, "model name") == 0)
			sib->cpu_model_name = sy_cpu_model(value);
		else if (strcmp(key, "vendor") == 0)
			sib->cpu_vendor = HX_strdup(value);
		else if (strcmp(key, "arch") == 0)
			sib->cpu_arch = HX_strdup(value);
		else if (strcmp(key, "clock") == 0 ||
		    strcmp(key, "cpu MHz") == 0)
			sib->cpu_mhz = strtod(value, NULL);
		else if (sscanf(key, "Cpu%udClkTck", &dummy_uint) == 1)
			sib->cpu_mhz = strtoull(value, NULL, 16) / 1000000;
	}
	HXmc_free(line);
	fclose(fp);
}

static void sy_cpupower(struct sy_block *sib)
{
	struct HXdir *dh = HXdir_open(sysfs_cpu_dir);
	hxmc_t *line = NULL;
	const char *de;
	char path[80];
	FILE *fp;

	if (dh == NULL)
		return;
	while ((de = HXdir_read(dh)) != NULL) {
		if (strncmp(de, "cpu", 3) != 0)
			continue;
		snprintf(path, sizeof(path), "%s/%s/cpufreq/cpuinfo_max_freq",
		         sysfs_cpu_dir, de);
		fp = fopen(path, "r");
		if (fp == NULL)
			continue;
		if (HX_getl(&line, fp) != NULL) {
			/* this is in KHz */
			sib->cpu_mhz = strtoull(line, NULL, 0) / 1000;
			HXmc_free(line);
		}
		fclose(fp);
		break;
	}
	HXdir_close(dh);
}

static void sy_loadavg(struct sy_block *sib)
{
	double avg5, avg15;
	unsigned int run;
	FILE *fp = fopen("/proc/loadavg", "r");

	if (fp == NULL)
		return;
	fscanf(fp, "%lf %lf %lf %u/%u",
	       &sib->load_avg1, &avg5, &avg15, &run, &sib->num_tasks);
	fclose(fp);
}

static void sy_memory(struct sy_block *sib)
{
	unsigned long long mem_free = 0, mem_buf = 0, mem_cac = 0, mem_shm = 0;
	FILE *fp = fopen("/proc/meminfo", "r");
	hxmc_t *line = NULL;
	char *key, *value;

	if (fp == NULL)
		return;
	while (HX_getl(&line, fp) != NULL) {
		value = key = line;
		value = sy_hop_colon(key, value);
		if (value == NULL)
			continue;
		if (strcmp(key, "MemTotal") == 0)
			sib->mem_total = strtoull(value, NULL, 0);
		else if (strcmp(key, "MemFree") == 0)
			mem_free = strtoull(value, NULL, 0);
		else if (strcmp(key, "Buffers") == 0)
			mem_buf = strtoull(value, NULL, 0);
		else if (strcmp(key, "Cached") == 0)
			mem_cac = strtoull(value, NULL, 0);
		else if (strcmp(key, "Shmem") == 0)
			mem_shm = strtoull(value, NULL, 0);
	}
	sib->mem_used = sib->mem_total - mem_free - mem_buf - mem_cac + mem_shm;
	HXmc_free(line);
	fclose(fp);
}

static void sy_disk(struct sy_block *sib)
{
	struct libmnt_context *ctx;
	struct libmnt_table *table;
	struct libmnt_iter *iter;
	struct libmnt_fs *fs;
	struct HXmap *seen;

	seen = HXmap_init(HXMAPT_DEFAULT, HXMAP_SCDATA);
	if (seen == NULL)
		return;
	ctx = mnt_new_context();
	if (ctx == NULL)
		goto out;
	if (mnt_context_get_mtab(ctx, &table) != 0)
		goto out;
	iter = mnt_new_iter(MNT_ITER_FORWARD);
	if (iter == NULL)
		goto out;
	while (mnt_table_next_fs(table, iter, &fs) == 0) {
		const char *source = mnt_fs_get_source(fs);
		const char *mntpt  = mnt_fs_get_target(fs);

		if (source == NULL || *source != '/')
			continue;
		if (HXmap_add(seen, reinterpret_cast(const void *,
		    static_cast(uintptr_t, mnt_fs_get_devno(fs))), mntpt) < 0)
			goto out;
	}
	mnt_reset_iter(iter, MNT_ITER_FORWARD);
	while (mnt_table_next_fs(table, iter, &fs) == 0) {
		const char *mntpt, *saved_mntpt;
		struct statvfs sb;

		mntpt = mnt_fs_get_target(fs);
		saved_mntpt = HXmap_get(seen, reinterpret_cast(const void *,
			static_cast(uintptr_t, mnt_fs_get_devno(fs))));

		if (mntpt == NULL || *mntpt != '/')
			continue;
		if (saved_mntpt == NULL ||
		    strcmp(mntpt, saved_mntpt) != 0)
			continue;
		if (statvfs(mntpt, &sb) < 0)
			continue;
		sib->disk_used  += (sb.f_blocks - sb.f_bavail) * sb.f_bsize;
		sib->disk_total += sb.f_blocks * sb.f_bsize;
	}
 out:
	if (ctx != NULL)
		mnt_free_context(ctx);
	HXmap_free(seen);
}

static void sy_gfx_hardware(struct sy_block *sib)
{
	struct pci_dev *pd;
	struct pci_access *pacc;
	const char *ret = NULL;

	pacc = pci_alloc();
	if (pacc == NULL)
		return;
	pci_init(pacc);
	pci_scan_bus(pacc);
	for (pd = pacc->devices; pd != NULL; pd = pd->next) {
		if ((pd->device_class >> 8) != PCI_BASE_CLASS_DISPLAY)
			continue;
		break;
	}
	if (pd != NULL)
		ret = pci_lookup_name(pacc, sib->gfx_hardware_buf,
		      sizeof(sib->gfx_hardware_buf),
		      PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
		      pd->vendor_id, pd->device_id);
	pci_cleanup(pacc);
	sib->gfx_hardware = ret;
}

static void sy_display_size(struct sy_block *sib)
{
	xcb_connection_t *conn;
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	const xcb_screen_t *screen;

	conn = xcb_connect(NULL, NULL);
	if (conn == NULL)
		return;
	setup = xcb_get_setup(conn);
	if (setup == NULL)
		goto out;
	iter = xcb_setup_roots_iterator(setup);
	screen = iter.data;
	sib->display_width  = screen->width_in_pixels;
	sib->display_height = screen->height_in_pixels;
 out:
	xcb_disconnect(conn);
}

static void sy_dump(const struct sy_block *sib)
{
	/* system */
	printf("(by hxtools sysinfo) [%s] %s %s %s",
		sib->uts.nodename, sib->uts.sysname, sib->uts.release,
		sib->uts.machine);

	/* cpu */
	if (sib->num_cpu_threads == 0)
		printf(" |");
	else
		printf(" | %u-thr", sib->num_cpu_threads);
	if (sib->cpu_model_name != NULL) {
		printf(" %s", sib->cpu_model_name);
	} else {
		if (sib->cpu_vendor != NULL)
			printf(" %s", sib->cpu_vendor);
		if (sib->cpu_arch != NULL)
			printf(" %s", sib->cpu_arch);
	}
	if (sib->cpu_mhz != 0)
		printf(" %.0fMHz", sib->cpu_mhz);

	/* load */
	printf(" | Load: %.2f Tasks: %u", sib->load_avg1, sib->num_tasks);

	/* memory */
	printf(" | Mem: %llu/%lluMB", sib->mem_used / 1024,
	       sib->mem_total / 1024);

	/* disk */
	printf(" | Disk: %llu/%lluGB",
	       sib->disk_used / (1024 * 1048576),
	       sib->disk_total / (1024 * 1048576));

	/* gfx */
	if (sib->gfx_hardware != NULL || sib->display_width != 0 ||
	    sib->display_height != 0) {
		printf(" | Gfx:");
		if (sib->gfx_hardware != NULL)
			printf(" %s", sib->gfx_hardware);
		if (sib->display_width != 0 || sib->display_height != 0)
			printf(" @ %ux%u", sib->display_width,
			       sib->display_height);
	}
	printf("\n");
}

static bool sy_get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		{.sh = 'P', .type = HXTYPE_STRING, .ptr = &sy_cpuinfo_file,
		 .help = "Debug: specify alternate cpuinfo file", .htyp = "FILE"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) !=
	    HXOPT_ERR_SUCCESS)
		return false;
	return true;
}

int main(int argc, const char **argv)
{
	struct sy_block sib;

	if (!sy_get_options(&argc, &argv))
		return EXIT_FAILURE;

	memset(&sib, 0, sizeof(sib));
	uname(&sib.uts);
	sy_num_cpu_threads(&sib);
	sy_proc_cpuinfo(&sib);
	sy_cpupower(&sib);
	sy_loadavg(&sib);
	sy_memory(&sib);
	sy_disk(&sib);
	sy_gfx_hardware(&sib);
	sy_display_size(&sib);
	sy_dump(&sib);
	return EXIT_SUCCESS;
}
