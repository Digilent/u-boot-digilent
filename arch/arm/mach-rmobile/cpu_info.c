// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * (C) Copyright 2012-2021 Renesas Solutions Corp.
 */

#include <cpu_func.h>
#include <asm/cache.h>
#include <init.h>
#include <asm/io.h>
#include <env.h>
#include <linux/ctype.h>

#ifdef CONFIG_ARCH_CPU_INIT
int arch_cpu_init(void)
{
	icache_enable();
	return 0;
}
#endif

/* R-Car Gen3 and Gen4 D-cache is enabled in memmap-gen3.c */
#ifndef CONFIG_RCAR_64
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	dcache_enable();
}
#endif
#endif

#ifdef CONFIG_DISPLAY_CPUINFO
#if !defined(CONFIG_RZA1) && !defined(CONFIG_RZN1)
__weak const u8 *rzg_get_cpu_name(void)
{
	return 0;
}

static u32 __rmobile_get_cpu_type(void)
{
	return 0x0;
}
u32 rmobile_get_cpu_type(void)
		__attribute__((weak, alias("__rmobile_get_cpu_type")));

static u32 __rmobile_get_cpu_rev_integer(void)
{
	return 0;
}
u32 rmobile_get_cpu_rev_integer(void)
		__attribute__((weak, alias("__rmobile_get_cpu_rev_integer")));

static u32 __rmobile_get_cpu_rev_fraction(void)
{
	return 0;
}
u32 rmobile_get_cpu_rev_fraction(void)
		__attribute__((weak, alias("__rmobile_get_cpu_rev_fraction")));

/* CPU information table */
static const struct {
	u16 cpu_type;
	u8 cpu_name[10];
} rmobile_cpuinfo[] = {
	{ RMOBILE_CPU_TYPE_R8A7740, "R8A7740" },
	{ RMOBILE_CPU_TYPE_R8A7790, "R8A7790" },
	{ RMOBILE_CPU_TYPE_R8A7791, "R8A7791" },
	{ RMOBILE_CPU_TYPE_R8A7792, "R8A7792" },
	{ RMOBILE_CPU_TYPE_R8A7793, "R8A7793" },
	{ RMOBILE_CPU_TYPE_R8A7794, "R8A7794" },
	{ RMOBILE_CPU_TYPE_R8A7795, "R8A7795" },
	{ RMOBILE_CPU_TYPE_R8A7796, "R8A7796" },
	{ RMOBILE_CPU_TYPE_R8A77965, "R8A77965" },
	{ RMOBILE_CPU_TYPE_R8A77970, "R8A77970" },
	{ RMOBILE_CPU_TYPE_R8A77980, "R8A77980" },
	{ RMOBILE_CPU_TYPE_R8A77990, "R8A77990" },
	{ RMOBILE_CPU_TYPE_R8A77995, "R8A77995" },
	{ RMOBILE_CPU_TYPE_R8A779A0, "R8A779A0" },
	{ RMOBILE_CPU_TYPE_R8A779F0, "R8A779F0" },
	{ RMOBILE_CPU_TYPE_R8A779G0, "R8A779G0" },
	{ 0x0, "CPU" },
};

static int rmobile_cpuinfo_idx(void)
{
	u32 cpu_type = rmobile_get_cpu_type();
	int i;

	for (i = 0; i < ARRAY_SIZE(rmobile_cpuinfo) - 1; i++)
		if (rmobile_cpuinfo[i].cpu_type == cpu_type)
			return i;

	/* Unknown "CPU" entry */
	return ARRAY_SIZE(rmobile_cpuinfo) - 1;
}

static const u8 *get_cpu_name(int idx)
{
	const  u8 *cpu_name = rzg_get_cpu_name();

	return cpu_name ? cpu_name : rmobile_cpuinfo[idx].cpu_name;
}

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	int i, idx = rmobile_cpuinfo_idx();
	const u8 *cpu_name = get_cpu_name(idx);
	char cpu[10] = { 0 };

	for (i = 0; i < sizeof(cpu); i++)
		cpu[i] = tolower(cpu_name[i]);

	env_set("platform", cpu);

	return 0;
}
#endif

int print_cpuinfo(void)
{
	int i = rmobile_cpuinfo_idx();

	if (rmobile_cpuinfo[i].cpu_type == RMOBILE_CPU_TYPE_R8A7796 &&
	    rmobile_get_cpu_rev_integer() == 1 &&
	    rmobile_get_cpu_rev_fraction() == 1) {
		printf("CPU:   Renesas Electronics %s rev 1.1/1.2\n", get_cpu_name(i));
		return 0;
	}

	printf("CPU:   Renesas Electronics %s rev %d.%d\n",
		get_cpu_name(i), rmobile_get_cpu_rev_integer(),
		rmobile_get_cpu_rev_fraction());

	return 0;
}
#elif defined(CONFIG_RZA1)
int print_cpuinfo(void)
{
	printf("CPU: Renesas Electronics RZ/A1\n");
	return 0;
}
#else /* CONFIG_RZN1 */
int print_cpuinfo(void)
{
	printf("CPU: Renesas Electronics RZ/N1\n");
	return 0;
}
#endif
#endif /* CONFIG_DISPLAY_CPUINFO */
