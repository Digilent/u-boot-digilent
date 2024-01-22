/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#ifndef __CONFIG_BMIPS_BCM6348_H
#define __CONFIG_BMIPS_BCM6348_H

#include <linux/sizes.h>

/* RAM */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* U-Boot */

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CFG_SYS_INIT_SP_OFFSET	SZ_8K
#endif

#define CFG_SYS_FLASH_BASE			0xbfc00000

#endif /* __CONFIG_BMIPS_BCM6348_H */
