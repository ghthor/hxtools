/*
 *	proc_stat_parse - turn /proc/N/stat into human-readable form
 *	written by Jan Engelhardt <jengelh [at] medozas de>, 2012
 *	placed into the Public Domain
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX/init.h>
#include <libHX/string.h>

struct sp_task {
	char comm[31], state;
	int pid, ppid, pgid, sid, tty_nr, tty_pgrp;
	unsigned int task_flags;
	/*
	 * We have to use unsigned long long here, since the kernel's unsigned
	 * long may be longer than userspace's.
	 */
	unsigned long long min_flt, cmin_flt, max_flt, cmax_flt;
	long long utime, stime, cutime, cstime;
	int priority, nice, num_threads;
	unsigned long long start_time, vsize, get_mm_rss, rsslim;
	unsigned long long mm_start_code, mm_end_code, mm_start_stack;
	unsigned long long sp, ip, sig_pend, sig_block, sig_ign, sig_catch;
	unsigned long long wchan;
	int exit_signal, task_cpu;
	unsigned int task_rt_prio, task_policy;
	unsigned long long da_blkio_ticks, gtime, cgtime, start_data;
	unsigned long long end_data, start_brk;
};

static void sp_init_task(struct sp_task *p)
{
	memset(p, 0, sizeof(*p));
}

static void sp_interp_line(const char *s)
{
	struct sp_task t;

	sp_init_task(&t);
	sscanf(s,/*1 */ "%d %31s %c %d %d "
	       /*  6 */ "%d %d %d %u %llu "
	       /* 11 */ "%llu %llu %llu %llu %llu "
	       /* 16 */ "%llu %llu %d %d %d "
	       /* 21 */ "0 %llu %llu %llu %llu "
	       /* 26 */ "%llu %llu %llu %llu %llu "
	       /* 31 */ "%llu %llu %llu %llu %llu "
	       /* 36 */ "0 0 %d %d %u "
	       /* 41 */ "%u %llu %llu %lld %llu "
	       /* 46 */ "%llu %llu",
	       /*  1 */ &t.pid, t.comm, &t.state, &t.ppid, &t.pgid,
	       /*  6 */ &t.sid, &t.tty_nr, &t.tty_pgrp, &t.task_flags, &t.min_flt,
	       /* 11 */ &t.cmin_flt, &t.max_flt, &t.cmax_flt, &t.utime, &t.stime,
	       /* 16 */ &t.cutime, &t.cstime, &t.priority, &t.nice, &t.num_threads,
	       /* 21 */ /*,*/ &t.start_time, &t.vsize, &t.get_mm_rss, &t.rsslim,
	       /* 26 */ &t.mm_start_code, &t.mm_end_code, &t.mm_start_stack, &t.sp, &t.ip,
	       /* 31 */ &t.sig_pend, &t.sig_block, &t.sig_ign, &t.sig_catch, &t.wchan,
	       /* 36 */ /*,,*/ &t.exit_signal, &t.task_cpu, &t.task_rt_prio,
	       /* 41 */ &t.task_policy, &t.da_blkio_ticks, &t.gtime, &t.cgtime, &t.start_data,
	       /* 46 */ &t.end_data, &t.start_brk
	       );
	printf("01 PID:             %d\n", t.pid);
	printf("02 Comm:            %s\n", t.comm);
	printf("03 State:           %c\n", t.state);
	printf("04 PPID:            %d\n", t.ppid);
	printf("05 PGID:            %d\n", t.pgid);
	printf("06 SID:             %d\n", t.sid);
	printf("07 tty_nr:          %d\n", t.tty_nr);
	printf("08 tty_pgrp:        %d\n", t.tty_pgrp);
	printf("09 task_flags:            0x%x\n", t.task_flags);
	printf("10 min_flt:         %llu\n", t.min_flt);
	printf("11 cmin_flt:        %llu\n", t.cmin_flt);
	printf("12 max_flt:         %llu\n", t.max_flt);
	printf("13 cmax_flt:        %llu\n", t.cmax_flt);
	printf("14 utime:           %lld clocks (= %lld s)\n",
	       t.utime, t.utime / CLOCKS_PER_SEC);
	printf("15 stime:           %lld clocks (= %lld s)\n",
	       t.stime, t.stime / CLOCKS_PER_SEC);
	printf("16 cutime:          %lld clocks (= %lld s)\n",
	       t.cutime, t.cutime / CLOCKS_PER_SEC);
	printf("17 cstime:          %lld clocks (= %lld s)\n",
	       t.cstime, t.cstime / CLOCKS_PER_SEC);
	printf("18 priority:        %d\n", t.priority);
	printf("19 nice:            %d\n", t.nice);
	printf("20 num_threads:     %d\n", t.num_threads);
	printf("21 --                     always 0\n");
	printf("22 start_time:      %llu\n", t.start_time);
	printf("23 vsize:           0x%llx (%llu)\n", t.vsize, t.vsize);
	printf("24 get_mm_rss:      0x%llx (%llu)\n", t.get_mm_rss, t.get_mm_rss);
	printf("25 rsslim:          0x%llx (%llu)\n", t.rsslim, t.rsslim);
	printf("26 mm_start_code:   0x%llx\n", t.mm_start_code);
	printf("27 mm_end_code:     0x%llx\n", t.mm_end_code);
	printf("28 mm_start_stack:  0x%llx\n", t.mm_start_stack);
	printf("29 sp register:     0x%llx\n", t.sp);
	printf("30 ip register:     0x%llx\n", t.ip);
	printf("31 obsol-sig-pend:  0x%llx\n", t.sig_pend);
	printf("32 obsol-sig-block: 0x%llx\n", t.sig_block);
	printf("33 obsol-sig-ign:   0x%llx\n", t.sig_ign);
	printf("34 obsol-sig-catch: 0x%llx\n", t.sig_catch);
	printf("35 wchan:           0x%llx\n", t.wchan);
	printf("36 --               always 0\n");
	printf("37 --               always 0\n");
	printf("38 task_exit_sig:   %d\n", t.exit_signal);
	printf("39 task_cpu:        %d\n", t.task_cpu);
	printf("40 task_rt_prio:    %u\n", t.task_rt_prio);
	printf("41 task_policy:     %u\n", t.task_policy);
	printf("42 delayacct_blkio: %llu ticks\n", t.da_blkio_ticks);
	printf("43 gtime:           %lld clocks (= %lld s)\n",
	       t.gtime, t.gtime / CLOCKS_PER_SEC);
	printf("44 cgtime:          %lld clocks (= %lld s)\n",
	       t.cgtime, t.cgtime / CLOCKS_PER_SEC);
	printf("45 start_data:      0x%llx\n", t.start_data);
	printf("46 end_data:        0x%llx\n", t.end_data);
	printf("47 start_brk:       0x%llx\n", t.start_brk);
}

static void sp_interp_file(FILE *fp)
{
	hxmc_t *line = NULL;

	while (HX_getl(&line, fp) != NULL)
		sp_interp_line(line);

	HXmc_free(line);
}

int main(int argc, const char **argv)
{
	const char *prog = argv[0];
	int ret;

	ret = HX_init();
	if (ret <= 0) {
		fprintf(stderr, "%s: HX_init: %s\n", prog, strerror(-ret));
		return EXIT_FAILURE;
	}
	if (argc == 1) {
		sp_interp_file(stdin);
		HX_exit();
		return EXIT_SUCCESS;
	}
	while (*++argv != NULL) {
		FILE *fp = fopen(*argv, "r");
		if (fp == NULL) {
			fprintf(stderr, "%s: %s: %s\n",
			        prog, *argv, strerror(errno));
			continue;
		}
		sp_interp_file(fp);
		fclose(fp);
	}		
	HX_exit();
	return EXIT_SUCCESS;
}
