
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

	printf("%s: Flash Device name: %s\n", __func__, flash->name);

#if defined (CONFIG_ZYNQ_GEM_MAC_IN_OTP)

	#if defined (CONFIG_QSPI_IS_SM) // For Spansion and Micron OPCODE
		#define CMD_OTP_READ_ARRAY_FAST 0x4b
	#elif defined (CONFIG_QSPI_IS_WB) // For Winbond OPCODE
		#define CMD_OTP_READ_ARRAY_FAST 0x48
	#else
		#define CMD_OTP_READ_ARRAY_FAST "NOT IMPLEMENTED"
	#endif

	//change read opcode only when OTP is used
	flash->read_opcode = CMD_OTP_READ_ARRAY_FAST;
#endif

	printf("%s: Reading SPI MAC address from offset %08lxh\n", __func__,
	       CONFIG_ZYNQ_GEM_MAC_QSPI_OFFSET);

	offset = CONFIG_ZYNQ_GEM_MAC_QSPI_OFFSET;

	if ((ret = spi_flash_read(flash, offset, 6, ethaddr)))
		printf("%s:SPI MAC address read failed\n", __func__);
	else
		printf("%s: SPI ethaddr: %pM\n", __func__, ethaddr);

	return ret;
}
