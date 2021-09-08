// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2019 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <spi.h>
#include <spi_flash.h>
#include "../../../drivers/mtd/spi/sf_internal.h"
#include "board.h"

#if defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET)
int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
	int ret = -EINVAL;

	struct udevice *dev;
	ofnode eeprom;

	eeprom = ofnode_get_chosen_node("xlnx,eeprom");
	if (!ofnode_valid(eeprom))
		return -ENODEV;

	debug("%s: Path to EEPROM %s\n", __func__,
	      ofnode_get_chosen_prop("xlnx,eeprom"));

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, eeprom, &dev);
	if (ret)
		return ret;
      
      	ret = dm_i2c_read(dev, CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET, ethaddr, 6);
        if (ret)
                debug("%s: I2C EEPROM MAC address read failed\n", __func__);
        else
                debug("%s: I2C EEPROM MAC %pM\n", __func__, ethaddr);

        return ret;
}
#endif

#if defined(CONFIG_ZYNQ_QSPI) && defined(CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET)
#if defined(CONFIG_ZYNQ_GEM_SPI_MAC_OTP)
#define CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST 0x4b
#define CMD_OTP_WINBOND_READ_ARRAY_FAST 0x48
#define SPI_OTP_OFFSET_SPANSION_MICRON 0x20
#define SPI_OTP_OFFSET_WINBOND 0x1000
#define ID_S25FL128S 0x012018
#define ID_MT25QL128ABA_N25Q128A 0x20BA18
#define ID_W25Q128JV 0xEF7018
#endif
int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
	int ret = -EINVAL;
	u32 offset;
	struct spi_flash *flash;
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);

	if (!flash) {
		printf("no flash\n");
		printf("SPI(bus:%u cs:%u) probe failed\n",
			CONFIG_SF_DEFAULT_BUS,
			CONFIG_SF_DEFAULT_CS);
		return 0;
	}
	debug("%s: Flash Device ID: %02Xh %02Xh %02Xh\n", __func__,
	flash->info->id[0], flash->info->id[1], flash->info->id[2]);

#if defined(CONFIG_ZYNQ_GEM_SPI_MAC_OTP)
	u32 id = ((u32)flash->info->id[0]<<16)|((u32)flash->info->id[1]<<8)|(u32)flash->info->id[2];
	if (id == ID_S25FL128S) {
		debug("%s: SPI Flash Spansion identified. Reading SPI MAC address from OTP area\n", __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset = SPI_OTP_OFFSET_SPANSION_MICRON + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST;
	}
	else if (id == ID_MT25QL128ABA_N25Q128A) {
		debug("%s: SPI Flash Micron identified. Reading SPI MAC address from OTP area\n", __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset = SPI_OTP_OFFSET_SPANSION_MICRON + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST;
	}
	else if (id == ID_W25Q128JV) {
		debug("%s: SPI Flash Winbond identified. Reading SPI MAC address from OTP area\n", __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset = SPI_OTP_OFFSET_WINBOND + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_WINBOND_READ_ARRAY_FAST;
	}
	else {
		printf("%s: Reading SPI MAC address from OTP area is not supported on this flash\n", __func__);
		return -EINVAL;
	}
#else
	debug("%s: Reading SPI MAC address from offset %08lxh\n", __func__, CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET);
        /*
         * Set the SPI MAC offset
         */
        offset = CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
#endif

	if ((ret = spi_flash_read(flash, offset, 6, ethaddr)))
		printf("%s:SPI MAC address read failed\n", __func__);
	else
		printf("%s: SPI ethaddr: %pM\n", __func__, ethaddr);

	return ret;
}
#endif

#if defined(CONFIG_OF_BOARD) || defined(CONFIG_OF_SEPARATE)
void *board_fdt_blob_setup(void)
{
	static void *fdt_blob;

#if !defined(CONFIG_VERSAL_NO_DDR) && !defined(CONFIG_ZYNQMP_NO_DDR)
	fdt_blob = (void *)CONFIG_XILINX_OF_BOARD_DTB_ADDR;

	if (fdt_magic(fdt_blob) == FDT_MAGIC)
		return fdt_blob;

	debug("DTB is not passed via %p\n", fdt_blob);
#endif

#ifdef CONFIG_SPL_BUILD
	/* FDT is at end of BSS unless it is in a different memory region */
	if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS))
		fdt_blob = (ulong *)&_image_binary_end;
	else
		fdt_blob = (ulong *)&__bss_end;
#else
	/* FDT is at end of image */
	fdt_blob = (ulong *)&_end;
#endif

	if (fdt_magic(fdt_blob) == FDT_MAGIC)
		return fdt_blob;

	debug("DTB is also not passed via %p\n", fdt_blob);

	return NULL;
}
#endif

int board_late_init_xilinx(void)
{
	bd_t *bd = gd->bd;

	if (bd->bi_dram[0].start) {
		ulong scriptaddr;

		scriptaddr = env_get_hex("scriptaddr", 0);
		env_set_hex("scriptaddr", bd->bi_dram[0].start + scriptaddr);
	}

	env_set_hex("script_offset_f", CONFIG_BOOT_SCRIPT_OFFSET);

	return 0;
}
