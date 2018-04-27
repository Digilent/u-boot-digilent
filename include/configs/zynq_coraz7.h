/*
 * (C) Copyright 2016 Digilent Inc.
 *
 * Configuration for Zynq Development Board - Cora Z7
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_CORAZ7_H
#define __CONFIG_ZYNQ_CORAZ7_H

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_USB
#define CONFIG_ZYNQ_I2C0

/* Define Cora Z7 PS Clock Frequency to 50MHz */
#define CONFIG_ZYNQ_PS_CLK_FREQ	50000000UL

#include <configs/zynq-common.h>

#endif /* __CONFIG_ZYNQ_CORAZ7_H */
