/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/stout.h
 *     This file is Stout board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Europe GmbH
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Cogent Embedded, Inc.
 */

#ifndef __STOUT_H
#define __STOUT_H

#include "rcar-gen2-common.h"

#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(1024u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(512 * 1024 * 1024)

/* SCIF */
#define CFG_SCIF_A

/* SH Ether */
#define CFG_SH_ETHER_USE_PORT	0
#define CFG_SH_ETHER_PHY_ADDR	0x1
#define CFG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CFG_SH_ETHER_CACHE_WRITEBACK
#define CFG_SH_ETHER_CACHE_INVALIDATE
#define CFG_SH_ETHER_ALIGNE_SIZE	64

/* Board Clock */

#define CFG_EXTRA_ENV_SETTINGS	\
	"bootm_size=0x10000000\0"

/* SPL support */

#endif	/* __STOUT_H */
