/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <fpga.h>
#include <mmc.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
static xilinx_desc fpga;

/* It can be done differently */
static xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
static xilinx_desc fpga015 = XILINX_XC7Z015_DESC(0x15);
static xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
static xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
static xilinx_desc fpga035 = XILINX_XC7Z035_DESC(0x35);
static xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
static xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

int board_init(void)
{
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7015:
		fpga = fpga015;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7035:
		fpga = fpga035;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init(void)
{
	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_NOR:
		setenv("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		setenv("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: Xilinx Zynq\n");
	return 0;
}
#endif

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
#if defined(CONFIG_ZYNQ_GEM_EEPROM_ADDR) && \
    defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET)
	if (eeprom_read(CONFIG_ZYNQ_GEM_EEPROM_ADDR,
			CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET,
			ethaddr, 6))
		printf("I2C EEPROM MAC address read failed\n");
#endif

	return 0;
}

int dram_init(void)
{
	int node;
	fdt_addr_t addr;
	fdt_size_t size;
	const void *blob = gd->fdt_blob;

	node = fdt_node_offset_by_prop_value(blob, -1, "device_type",
					     "memory", 7);
	if (node == -FDT_ERR_NOTFOUND) {
		debug("ZYNQ DRAM: Can't get memory node\n");
		return -1;
	}
	addr = fdtdec_get_addr_size(blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE || size == 0) {
		debug("ZYNQ DRAM: Can't get base address or size\n");
		return -1;
	}
	gd->ram_size = size;
	zynq_ddrc_init();

	return 0;
}
