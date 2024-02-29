
#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <log.h>
#include <bootdev.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/errno.h>
#include <i2c.h>
#include <net.h>
#include <spi.h>
#include <spi_flash.h>

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
	int ret = -EINVAL;
	u32 offset;
	struct spi_flash *flash;
	flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
	                        CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);

	if (!flash) {
		printf("no flash\n");
		printf("SPI(bus:%u cs:%u) probe failed\n", CONFIG_SF_DEFAULT_BUS,
		       CONFIG_SF_DEFAULT_CS);
		return 0;
	}

#if defined(ZYNQ_GEM_MAC_IN_OTP)

	NOT VERIFIED
	    // use #include "../../../drivers/mtd/spi/sf_internal.h" to access
	    // flash->id

		// or try to match flash->name instead of flash->info-> id??
	    printf("%s: Flash Device name: %s\n", __func__, flash->name);

	printf("%s: Flash Device ID: %02Xh %02Xh %02Xh\n", __func__,
	       flash->info->id[0], flash->info->id[1], flash->info->id[2]);

#define ID_S25FL128S             0x012018
#define ID_MT25QL128ABA_N25Q128A 0x20BA18
#define ID_W25Q128JV             0xEF7018

#define CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST 0x4b
#define CMD_OTP_WINBOND_READ_ARRAY_FAST         0x48

#define SPI_OTP_OFFSET_SPANSION_MICRON 0x20
#define SPI_OTP_OFFSET_WINBOND         0x1000

	u32 id = ((u32)flash->info->id[0] << 16) | ((u32)flash->info->id[1] << 8) |
	         (u32)flash->info->id[2];
	if (id == ID_S25FL128S) {
		debug("%s: SPI Flash Spansion identified. Reading SPI MAC address from "
		      "OTP area\n",
		      __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset =
		    SPI_OTP_OFFSET_SPANSION_MICRON + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST;
	} else if (id == ID_MT25QL128ABA_N25Q128A) {
		debug("%s: SPI Flash Micron identified. Reading SPI MAC address from "
		      "OTP area\n",
		      __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset =
		    SPI_OTP_OFFSET_SPANSION_MICRON + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_SPANSION_MICRON_READ_ARRAY_FAST;
	} else if (id == ID_W25Q128JV) {
		debug("%s: SPI Flash Winbond identified. Reading SPI MAC address from "
		      "OTP area\n",
		      __func__);
		/*
		 * Set the SPI MAC offset
		 */
		offset = SPI_OTP_OFFSET_WINBOND + CONFIG_ZYNQ_GEM_SPI_MAC_OFFSET;
		/*
		 * Set the cmd to otp read
		 */
		flash->read_opcode = CMD_OTP_WINBOND_READ_ARRAY_FAST;
	} else {
		printf("%s: Reading SPI MAC address from OTP area is not supported on "
		       "this flash\n",
		       __func__);
		return -EINVAL;
	}
#else
	printf("%s: Reading SPI MAC address from offset %08lxh\n", __func__,
	       CONFIG_ZYNQ_GEM_ETH_MAC_QSPI_OFFSET);
	/*
	 * Set the SPI MAC offset
	 */
	offset = CONFIG_ZYNQ_GEM_ETH_MAC_QSPI_OFFSET;
#endif

	if ((ret = spi_flash_read(flash, offset, 6, ethaddr)))
		printf("%s:SPI MAC address read failed\n", __func__);
	else
		printf("%s: SPI ethaddr: %pM\n", __func__, ethaddr);

	return ret;
}
