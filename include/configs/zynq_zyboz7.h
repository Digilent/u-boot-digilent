/*
 * (C) Copyright 2016 Digilent Inc.
 *
 * Configuration for Zynq Development Board - Zybo Z7
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ZYBOZ7_H
#define __CONFIG_ZYNQ_ZYBOZ7_H

#define CONFIG_SYS_NO_FLASH
#define CONFIG_ZYNQ_QSPI

#define CONFIG_ZYNQ_USB
#define CONFIG_ZYNQ_I2C0

/* GEM MAC address offset */
#define CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET	0x20

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_ZYBOZ7_H */
