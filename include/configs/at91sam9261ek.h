/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9261EK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768		/* slow clock xtal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	18432000	/* 18.432 MHz crystal */

#include <asm/hardware.h>

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000
#define CONFIG_SYS_INIT_RAM_SIZE	(16 * 1024)
#define CONFIG_SYS_INIT_RAM_ADDR	ATMEL_BASE_SRAM

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_BASE			0x40000000
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD22 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 22)
/* our CLE is AD21 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 21)
#define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIN_PC14
#define CONFIG_SYS_NAND_READY_PIN		AT91_PIN_PC15

#endif

/* Ethernet */
#define CONFIG_DM9000_BASE		0x30000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE + 4)
#define CONFIG_DM9000_USE_16BIT
#define CONFIG_DM9000_NO_SROM

/* USB */
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00500000	/* AT91SAM9261_UHP_BASE */

#endif
