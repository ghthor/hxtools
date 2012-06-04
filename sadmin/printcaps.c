/*
 *	Print current process capabilities in human-readable fashion
 *	written by Jan Engelhardt, 2011
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the WTF Public License version 2 or
 *	(at your option) any later version.
 */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/string.h>
#undef _POSIX_SOURCE
#include <sys/capability.h>

static const char *const cap_names[] = {
#define E(x) [CAP_##x] = #x
	E(CHOWN),
	E(DAC_OVERRIDE),
	E(DAC_READ_SEARCH),
	E(FOWNER),
	E(FSETID),
	E(KILL),
	E(SETGID),
	E(SETUID),
	E(SETPCAP),
	E(LINUX_IMMUTABLE),
	E(NET_BIND_SERVICE),
	E(NET_BROADCAST),
	E(NET_ADMIN),
	E(NET_RAW),
	E(IPC_LOCK),
	E(IPC_OWNER),
	E(SYS_MODULE),
	E(SYS_RAWIO),
	E(SYS_CHROOT),
	E(SYS_PTRACE),
	E(SYS_PACCT),
	E(SYS_ADMIN),
	E(SYS_BOOT),
	E(SYS_NICE),
	E(SYS_RESOURCE),
	E(SYS_TIME),
	E(SYS_TTY_CONFIG),
	E(MKNOD),
	E(LEASE),
	E(AUDIT_WRITE),
	E(AUDIT_CONTROL),
#ifdef CAP_SETFCAP
	E(SETFCAP),
#endif
#ifdef CAP_MAC_OVERRIDE
	E(MAC_OVERRIDE),
#endif
#ifdef CAP_MAC_ADMIN
	E(MAC_ADMIN),
#endif
#ifdef CAP_SYSLOG
	E(SYSLOG),
#endif
#ifdef CAP_WAKE_ALARM
	E(WAKE_ALARM),
#endif
#undef E
};

static int print_caps(cap_t data)
{
	cap_flag_value_t value;
	unsigned int i, j;

	printf("%-20s %s %s %s %-20s %s %s %s\n", "", "EFF", "PRM", "INH",
	                                          "", "EFF", "PRM", "INH");
	for (i = 0; i < (ARRAY_SIZE(cap_names) + 1) / 2; ++i) {
		j = i + ARRAY_SIZE(cap_names) / 2;

		printf("%-20s ", cap_names[i]);
		cap_get_flag(data, i, CAP_EFFECTIVE, &value);
		printf(" %s  ", (value == CAP_SET) ? "X" : ".");
		cap_get_flag(data, i, CAP_PERMITTED, &value);
		printf(" %s  ", (value == CAP_SET) ? "X" : ".");
		cap_get_flag(data, i, CAP_INHERITABLE, &value);
		printf(" %s  ", (value == CAP_SET) ? "X" : ".");

		if (j < ARRAY_SIZE(cap_names)) {
			printf("%-20s ", cap_names[j]);
			cap_get_flag(data, j, CAP_EFFECTIVE, &value);
			printf(" %s  ", (value == CAP_SET) ? "X" : ".");
			cap_get_flag(data, j, CAP_PERMITTED, &value);
			printf(" %s  ", (value == CAP_SET) ? "X" : ".");
			cap_get_flag(data, j, CAP_INHERITABLE, &value);
			printf(" %s", (value == CAP_SET) ? "X" : ".");
		}

		printf("\n");
	}

	cap_free(data);
	return EXIT_SUCCESS;
}

static int r_current(void)
{
	cap_t data;

	data = cap_get_proc();
	print_caps(data);
	return EXIT_SUCCESS;
}

static int r_show(unsigned int pid)
{
	cap_t data;

	data = cap_get_pid(pid);
	print_caps(data);
	return EXIT_SUCCESS;
}

int main(int argc, const char **argv)
{
	int ret;

	if ((ret = HX_init()) < 0) {
		fprintf(stderr, "%s\n", strerror(-ret));
		return EXIT_FAILURE;
	}
	if (argc == 1)
		ret = r_current();
	else
		while (*++argv != NULL)
			ret |= r_show(strtoul(*argv, NULL, 0));
	HX_exit();
	return ret;
}
