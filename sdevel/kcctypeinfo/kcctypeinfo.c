/*
 *	Print architecture characteristics - Linux kernel module variant
 *	written by Jan Engelhardt, 2011
 *	placed into the Public Domain
 */
#include <linux/module.h>
#include <linux/nls.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/in6.h>

struct x16 {
	uint8_t a;
	uint16_t b;
};

struct x32 {
	uint8_t a;
	uint16_t b;
	uint32_t c;
};

struct x64 {
	uint8_t a;
	uint16_t b;
	uint32_t c;
	uint64_t d;
};

#define p(type) q(type, type)
#define q(type, vname) \
	extern int SIZEOF_##vname, ALIGNOF_##vname; \
	int SIZEOF_##vname = sizeof(type), ALIGNOF_##vname = __alignof__(type);
#define t(type) \
	printk("%14s  %7zu  %7zu\n", #type, sizeof(type), __alignof__(type))

p(char);
p(short);
p(int);
p(long);
q(long long, longlong);
p(float);
p(double);
q(long double, longdouble);
q(void *, voidptr);
q(void (*)(void), funcptr);
p(uintptr_t);
p(wchar_t);
p(size_t);
p(off_t);
p(loff_t);
p(uint8_t);
p(uint16_t);
p(uint32_t);
p(uint64_t);
q(struct x16, x16);
q(struct x32, x32);
q(struct x64, x64);
p(mode_t);
p(time_t);
q(struct timespec, timespec);
q(struct sockaddr, sockaddr);
q(struct sockaddr_in, sockaddr_in);
q(struct sockaddr_in6, sockaddr_in6);

static int __init kcct_init(void)
{
	printk("%14s  %7s  %7s\n", "TYPE", "SIZEOF", "ALIGNOF");
	t(char);
	t(short);
	t(int);
	t(long);
	t(long long);
	t(float);
	t(double);
	t(long double);
	t(void *);
	t(void (*)(void));
	t(uintptr_t);
	t(size_t);
	t(wchar_t);
	t(off_t);
	t(loff_t);
	t(uint8_t);
	t(uint16_t);
	t(uint32_t);
	t(uint64_t);
	t(struct x16);
	t(struct x32);
	t(struct x64);
	t(mode_t);
	t(time_t);
	t(struct timespec);
	t(struct sockaddr);
	t(struct sockaddr_in);
	t(struct sockaddr_in6);
	t(struct sockaddr_storage);
	return -EIO;
}

static void __exit kcct_exit(void)
{
}

module_init(kcct_init);
module_exit(kcct_exit);
MODULE_LICENSE("GPL and additional rights");
