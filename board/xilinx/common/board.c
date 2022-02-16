// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2020 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <env.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <linux/sizes.h>
#include <malloc.h>
#include "board.h"
#include <dm.h>
#include <i2c_eeprom.h>
#include <net.h>
#include <generated/dt.h>
#include <soc.h>
#include <linux/ctype.h>

#include "fru.h"

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
	      ofnode_read_chosen_string("xlnx,eeprom"));

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

#define EEPROM_HEADER_MAGIC		0xdaaddeed
#define EEPROM_HDR_MANUFACTURER_LEN	16
#define EEPROM_HDR_NAME_LEN		16
#define EEPROM_HDR_REV_LEN		8
#define EEPROM_HDR_SERIAL_LEN		20
#define EEPROM_HDR_NO_OF_MAC_ADDR	4
#define EEPROM_HDR_ETH_ALEN		ETH_ALEN

struct xilinx_board_description {
	u32 header;
	char manufacturer[EEPROM_HDR_MANUFACTURER_LEN + 1];
	char name[EEPROM_HDR_NAME_LEN + 1];
	char revision[EEPROM_HDR_REV_LEN + 1];
	char serial[EEPROM_HDR_SERIAL_LEN + 1];
	u8 mac_addr[EEPROM_HDR_NO_OF_MAC_ADDR][EEPROM_HDR_ETH_ALEN + 1];
};

static int highest_id = -1;
static struct xilinx_board_description *board_info;

#define XILINX_I2C_DETECTION_BITS	sizeof(struct fru_common_hdr)

/* Variable which stores pointer to array which stores eeprom content */
struct xilinx_legacy_format {
	char board_sn[18]; /* 0x0 */
	char unused0[14]; /* 0x12 */
	char eth_mac[6]; /* 0x20 */
	char unused1[170]; /* 0x26 */
	char board_name[11]; /* 0xd0 */
	char unused2[5]; /* 0xdc */
	char board_revision[3]; /* 0xe0 */
	char unused3[29]; /* 0xe3 */
};

static void xilinx_eeprom_legacy_cleanup(char *eeprom, int size)
{
	int i;
	char byte;

	for (i = 0; i < size; i++) {
		byte = eeprom[i];

		/* Remove all ffs and spaces */
		if (byte == 0xff || byte == ' ')
			eeprom[i] = 0;

		/* Convert strings to lower case */
		if (byte >= 'A' && byte <= 'Z')
			eeprom[i] = byte + 'a' - 'A';
	}
}

static int xilinx_read_eeprom_legacy(struct udevice *dev, char *name,
				     struct xilinx_board_description *desc)
{
	int ret, size;
	struct xilinx_legacy_format *eeprom_content;
	bool eth_valid = false;

	size = sizeof(*eeprom_content);

	eeprom_content = calloc(1, size);
	if (!eeprom_content)
		return -ENOMEM;

	debug("%s: I2C EEPROM read pass data at %p\n", __func__,
	      eeprom_content);

	ret = dm_i2c_read(dev, 0, (uchar *)eeprom_content, size);
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		free(eeprom_content);
		return ret;
	}

	xilinx_eeprom_legacy_cleanup((char *)eeprom_content, size);

	printf("Xilinx I2C Legacy format at %s:\n", name);
	printf(" Board name:\t%s\n", eeprom_content->board_name);
	printf(" Board rev:\t%s\n", eeprom_content->board_revision);
	printf(" Board SN:\t%s\n", eeprom_content->board_sn);

	eth_valid = is_valid_ethaddr((const u8 *)eeprom_content->eth_mac);
	if (eth_valid)
		printf(" Ethernet mac:\t%pM\n", eeprom_content->eth_mac);

	/* Terminating \0 chars ensure end of string */
	strcpy(desc->name, eeprom_content->board_name);
	strcpy(desc->revision, eeprom_content->board_revision);
	strcpy(desc->serial, eeprom_content->board_sn);
	if (eth_valid)
		memcpy(desc->mac_addr[0], eeprom_content->eth_mac, ETH_ALEN);

	desc->header = EEPROM_HEADER_MAGIC;

	free(eeprom_content);

	return ret;
}

static bool xilinx_detect_legacy(u8 *buffer)
{
	int i;
	char c;

	for (i = 0; i < XILINX_I2C_DETECTION_BITS; i++) {
		c = buffer[i];

		if (c < '0' || c > '9')
			return false;
	}

	return true;
}

static int xilinx_read_eeprom_fru(struct udevice *dev, char *name,
				  struct xilinx_board_description *desc)
{
	int i, ret, eeprom_size;
	u8 *fru_content;
	u8 id = 0;

	/* FIXME this is shortcut - if eeprom type is wrong it will fail */
	eeprom_size = i2c_eeprom_size(dev);

	fru_content = calloc(1, eeprom_size);
	if (!fru_content)
		return -ENOMEM;

	debug("%s: I2C EEPROM read pass data at %p\n", __func__,
	      fru_content);

	ret = dm_i2c_read(dev, 0, (uchar *)fru_content,
			  eeprom_size);
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		goto end;
	}

	fru_capture((unsigned long)fru_content);
	if (gd->flags & GD_FLG_RELOC || (_DEBUG && CONFIG_IS_ENABLED(DTB_RESELECT))) {
		printf("Xilinx I2C FRU format at %s:\n", name);
		ret = fru_display(0);
		if (ret) {
			printf("FRU format decoding failed.\n");
			goto end;
		}
	}

	if (desc->header == EEPROM_HEADER_MAGIC) {
		debug("Information already filled\n");
		ret = -EINVAL;
		goto end;
	}

	/* It is clear that FRU was captured and structures were filled */
	strncpy(desc->manufacturer, (char *)fru_data.brd.manufacturer_name,
		sizeof(desc->manufacturer));
	strncpy(desc->name, (char *)fru_data.brd.product_name,
		sizeof(desc->name));
	for (i = 0; i < sizeof(desc->name); i++) {
		if (desc->name[i] == ' ')
			desc->name[i] = '\0';
	}
	strncpy(desc->revision, (char *)fru_data.brd.rev,
		sizeof(desc->revision));
	strncpy(desc->serial, (char *)fru_data.brd.serial_number,
		sizeof(desc->serial));

	while (id < EEPROM_HDR_NO_OF_MAC_ADDR) {
		if (is_valid_ethaddr((const u8 *)fru_data.mac.macid[id]))
			memcpy(&desc->mac_addr[id],
			       (char *)fru_data.mac.macid[id], ETH_ALEN);
		id++;
	}

	desc->header = EEPROM_HEADER_MAGIC;

end:
	free(fru_content);
	return ret;
}

static bool xilinx_detect_fru(u8 *buffer)
{
	u8 checksum = 0;
	int i;

	checksum = fru_checksum((u8 *)buffer, sizeof(struct fru_common_hdr));
	if (checksum) {
		debug("%s Common header CRC FAIL\n", __func__);
		return false;
	}

	bool all_zeros = true;
	/* Checksum over all zeros is also zero that's why detect this case */
	for (i = 0; i < sizeof(struct fru_common_hdr); i++) {
		if (buffer[i] != 0)
			all_zeros = false;
	}

	if (all_zeros)
		return false;

	debug("%s Common header CRC PASS\n", __func__);
	return true;
}

static int xilinx_read_eeprom_single(char *name,
				     struct xilinx_board_description *desc)
{
	int ret;
	struct udevice *dev;
	ofnode eeprom;
	u8 buffer[XILINX_I2C_DETECTION_BITS];

	eeprom = ofnode_get_aliases_node(name);
	if (!ofnode_valid(eeprom))
		return -ENODEV;

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, eeprom, &dev);
	if (ret)
		return ret;

	ret = dm_i2c_read(dev, 0, buffer, sizeof(buffer));
	if (ret) {
		debug("%s: I2C EEPROM read failed\n", __func__);
		return ret;
	}

	debug("%s: i2c memory detected: %s\n", __func__, name);

	if (CONFIG_IS_ENABLED(CMD_FRU) && xilinx_detect_fru(buffer))
		return xilinx_read_eeprom_fru(dev, name, desc);

	if (xilinx_detect_legacy(buffer))
		return xilinx_read_eeprom_legacy(dev, name, desc);

	return -ENODEV;
}

__maybe_unused int xilinx_read_eeprom(void)
{
	int id;
	char name_buf[8]; /* 8 bytes should be enough for nvmem+number */
	struct xilinx_board_description *desc;

	highest_id = dev_read_alias_highest_id("nvmem");
	/* No nvmem aliases present */
	if (highest_id < 0)
		return -EINVAL;

	board_info = calloc(1, sizeof(*desc) * (highest_id + 1));
	if (!board_info)
		return -ENOMEM;

	debug("%s: Highest ID %d, board_info %p\n", __func__,
	      highest_id, board_info);

	for (id = 0; id <= highest_id; id++) {
		snprintf(name_buf, sizeof(name_buf), "nvmem%d", id);

		/* Alloc structure */
		desc = &board_info[id];

		/* Ignoring return value for supporting multiple chips */
		xilinx_read_eeprom_single(name_buf, desc);
	}

	/*
	 * Consider to clean board_info structure when board/cards are not
	 * detected.
	 */

	return 0;
}

#if defined(CONFIG_OF_BOARD) || defined(CONFIG_OF_SEPARATE)
void *board_fdt_blob_setup(int *err)
{
	void *fdt_blob;

	*err = 0;
	if (!IS_ENABLED(CONFIG_SPL_BUILD) &&
	    !IS_ENABLED(CONFIG_VERSAL_NO_DDR) &&
	    !IS_ENABLED(CONFIG_ZYNQMP_NO_DDR)) {
		fdt_blob = (void *)CONFIG_XILINX_OF_BOARD_DTB_ADDR;

		if (fdt_magic(fdt_blob) == FDT_MAGIC)
			return fdt_blob;

		debug("DTB is not passed via %p\n", fdt_blob);
	}

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		/*
		 * FDT is at end of BSS unless it is in a different memory
		 * region
		 */
		if (IS_ENABLED(CONFIG_SPL_SEPARATE_BSS))
			fdt_blob = (ulong *)&_image_binary_end;
		else
			fdt_blob = (ulong *)&__bss_end;
	} else {
		/* FDT is at end of image */
		fdt_blob = (ulong *)&_end;
	}

	if (fdt_magic(fdt_blob) == FDT_MAGIC)
		return fdt_blob;

	debug("DTB is also not passed via %p\n", fdt_blob);

	return NULL;
}
#endif

#if defined(CONFIG_BOARD_LATE_INIT)
static int env_set_by_index(const char *name, int index, char *data)
{
	char var[32];

	if (!index)
		sprintf(var, "board_%s", name);
	else
		sprintf(var, "card%d_%s", index, name);

	return env_set(var, data);
}

int board_late_init_xilinx(void)
{
	u32 ret = 0;
	int i, id, macid = 0;
	struct xilinx_board_description *desc;
	phys_size_t bootm_size = gd->ram_size;

	if (!CONFIG_IS_ENABLED(MICROBLAZE)) {
		ulong scriptaddr;

		scriptaddr = env_get_hex("scriptaddr", 0);
		ret |= env_set_hex("scriptaddr", gd->ram_base + scriptaddr);
	}

	if (CONFIG_IS_ENABLED(ARCH_ZYNQ) || CONFIG_IS_ENABLED(MICROBLAZE))
		bootm_size = min(bootm_size, (phys_size_t)(SZ_512M + SZ_256M));

	ret |= env_set_hex("script_offset_f", CONFIG_BOOT_SCRIPT_OFFSET);

	ret |= env_set_addr("bootm_low", (void *)gd->ram_base);
	ret |= env_set_addr("bootm_size", (void *)bootm_size);

	for (id = 0; id <= highest_id; id++) {
		desc = &board_info[id];
		if (desc && desc->header == EEPROM_HEADER_MAGIC) {
			if (desc->manufacturer[0])
				ret |= env_set_by_index("manufacturer", id,
							desc->manufacturer);
			if (desc->name[0])
				ret |= env_set_by_index("name", id,
							desc->name);
			if (desc->revision[0])
				ret |= env_set_by_index("rev", id,
							desc->revision);
			if (desc->serial[0])
				ret |= env_set_by_index("serial", id,
							desc->serial);

			if (!CONFIG_IS_ENABLED(NET))
				continue;

			for (i = 0; i < EEPROM_HDR_NO_OF_MAC_ADDR; i++) {
				if (!desc->mac_addr[i])
					break;

				if (is_valid_ethaddr((const u8 *)desc->mac_addr[i]))
					ret |= eth_env_set_enetaddr_by_index("eth",
							macid++, desc->mac_addr[i]);
			}
		}
	}

	if (ret)
		printf("%s: Saving run time variables FAILED\n", __func__);

	return 0;
}
#endif

static char *board_name = DEVICE_TREE;

int __maybe_unused board_fit_config_name_match(const char *name)
{
	debug("%s: Check %s, default %s\n", __func__, name, board_name);

	if (!strcmp(name, board_name))
		return 0;

	return -1;
}

#if defined(CONFIG_DISPLAY_CPUINFO) && !defined(CONFIG_ARCH_ZYNQ)
int print_cpuinfo(void)
{
	struct udevice *soc;
	char name[SOC_MAX_STR_SIZE];
	int ret;

	ret = soc_get(&soc);
	if (ret) {
		printf("CPU:   UNKNOWN\n");
		return 0;
	}

	ret = soc_get_family(soc, name, SOC_MAX_STR_SIZE);
	if (ret)
		printf("CPU:   %s\n", name);

	ret = soc_get_revision(soc, name, SOC_MAX_STR_SIZE);
	if (ret)
		printf("Silicon: %s\n", name);

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(DTB_RESELECT)
#define MAX_NAME_LENGTH	50

char * __maybe_unused __weak board_name_decode(void)
{
	char *board_local_name;
	struct xilinx_board_description *desc;
	int i, id;

	board_local_name = calloc(1, MAX_NAME_LENGTH);
	if (!board_info)
		return NULL;

	for (id = 0; id <= highest_id; id++) {
		desc = &board_info[id];

		/* No board description */
		if (!desc)
			goto error;

		/* Board is not detected */
		if (desc->header != EEPROM_HEADER_MAGIC)
			continue;

		/* The first string should be soc name */
		if (!id)
			strcat(board_local_name, CONFIG_SYS_BOARD);

		/*
		 * For two purpose here:
		 * soc_name- eg: zynqmp-
		 * and between base board and CC eg: ..revA-sck...
		 */
		strcat(board_local_name, "-");

		if (desc->name[0]) {
			/* For DT composition name needs to be lowercase */
			for (i = 0; i < sizeof(desc->name); i++)
				desc->name[i] = tolower(desc->name[i]);

			strcat(board_local_name, desc->name);
		}
		if (desc->revision[0]) {
			strcat(board_local_name, "-rev");

			/* And revision needs to be uppercase */
			for (i = 0; i < sizeof(desc->revision); i++)
				desc->revision[i] = toupper(desc->revision[i]);

			strcat(board_local_name, desc->revision);
		}
	}

	/*
	 * Longer strings will end up with buffer overflow and potential
	 * attacks that's why check it
	 */
	if (strlen(board_local_name) >= MAX_NAME_LENGTH)
		panic("Board name can't be determined\n");

	if (strlen(board_local_name))
		return board_local_name;

error:
	free(board_local_name);
	return NULL;
}

bool __maybe_unused __weak board_detection(void)
{
	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM)) {
		int ret;

		ret = xilinx_read_eeprom();
		return !ret ? true : false;
	}

	return false;
}

int embedded_dtb_select(void)
{
	if (board_detection()) {
		char *board_local_name;

		board_local_name = board_name_decode();
		if (board_local_name) {
			board_name = board_local_name;
			printf("Detected name: %s\n", board_name);

			/* Time to change DTB on fly */
			/* Both ways should work here */
			/* fdtdec_resetup(&rescan); */
			fdtdec_setup();
		}
	}
	return 0;
}
#endif
