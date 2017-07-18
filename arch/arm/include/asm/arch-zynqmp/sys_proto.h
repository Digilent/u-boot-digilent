/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#define PAYLOAD_ARG_CNT		5

#ifndef CONFIG_CLK_ZYNQMP
/* Setup clk for network */
static inline void zynq_slcr_gem_clk_setup(u32 gem_id, unsigned long clk_rate)
{
}
#endif

int zynq_slcr_get_mio_pin_status(const char *periph);

unsigned int zynqmp_get_silicon_version(void);

void psu_init(void);

void handoff_setup(void);

void zynqmp_pmufw_version(void);
int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value);
int zynqmp_mmio_read(const u32 address, u32 *value);
int invoke_smc(u32 pm_api_id, u32 arg0, u32 arg1, u32 arg2, u32 arg3,
	       u32 *ret_payload);

#endif /* _ASM_ARCH_SYS_PROTO_H */
