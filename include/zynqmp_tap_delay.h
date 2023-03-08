/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#ifndef __ZYNQMP_TAP_DELAY_H__
#define __ZYNQMP_TAP_DELAY_H__

#ifdef CONFIG_ARCH_ZYNQMP
void zynqmp_dll_reset(u8 deviceid);
void arasan_zynqmp_set_tapdelay_w_disable(u8 device_id, u32 itap_delay, u32 otap_delay,
						u8 disable_dly);
void arasan_zynqmp_set_tapdelay(u8 device_id, u32 itap_delay, u32 otap_delay);
#else
inline void zynqmp_dll_reset(u8 deviceid) {}
inline void arasan_zynqmp_set_tapdelay(u8 device_id, u32 itap_delay,
				       u32 otap_delay) {}
#endif

#ifdef CONFIG_SD_TUNING_WORKAROUND
// For 200MHz clock, 30 Taps are available
#define SDR104_MAX_INPUT_TAPS 30
#endif

#endif
