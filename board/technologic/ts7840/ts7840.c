/*
 * Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <watchdog.h>
#include <status_led.h>

#include "../drivers/ddr/mv-ddr-marvell/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

#include "tsfpga.h"

DECLARE_GLOBAL_DATA_PTR;

#define TS7800_V2_SYSCON_BASE   (MBUS_PCI_MEM_BASE + 0x100000)
#define TS7800_V2_SYSCON_SIZE   0x48

#define ETH_PHY_CTRL_REG		0
#define ETH_PHY_CTRL_POWER_DOWN_BIT	11
#define ETH_PHY_CTRL_POWER_DOWN_MASK	(1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
};

volatile unsigned char *syscon_base;

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

static struct mv_ddr_topology_map ts7840_ecc_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_32BIT,	/* sdram device width */
	    MV_DDR_DIE_CAP_4GBIT,	/* die capacity */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    MV_DDR_TEMP_LOW} },		/* temperature */
	BUS_MASK_32BIT,		/* subphys mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	/* Return the board topology as defined in the board code */
	return &ts7840_ecc_topology_map;
}

void board_spi_cs_activate(int cs)
{
	if(cs == 1) {
		writel(0x1, 0xe8100028);
		writel(0x0, 0xe8100018);
	}
}

void board_spi_cs_deactivate(int cs)
{
	if(cs == 1) {
		writel(0x1, 0xe8100018);
	}
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00); /* 7:0 */
	writel(0x11111111, MVEBU_MPP_BASE + 0x04); /* 15:8 */
	writel(0x11120001, MVEBU_MPP_BASE + 0x08); /* 23:16 */
	writel(0x22222211, MVEBU_MPP_BASE + 0x0c); /* 31:24 */
	writel(0x22220000, MVEBU_MPP_BASE + 0x10); /* 39:32 */
	writel(0x00000004, MVEBU_MPP_BASE + 0x14); /* 47:40 */
	writel(0x55000500, MVEBU_MPP_BASE + 0x18); /* 55:48 */
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c); /* 63:56 */

	/* Set GPP Out value */
	writel(BOARD_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(BOARD_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(BOARD_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(BOARD_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(BOARD_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(BOARD_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_late_init(void)
{
	uint32_t dat;
	char tmp_buf[10];
	pci_dev_t dev;

	struct pci_device_id ids[2] = {
		{TS7840_FPGA_VENDOR, TS7840_FPGA_DEVICE},
		{0, 0}
	};

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_revision", "P1");
	env_set("board_name", "TS-7840");
	env_set("board_model", "7840");
#endif

	/* default, in case  we can't find it by a PCI scan */
	syscon_base = TS7840_SYSCON_BASE;

	if ((dev = pci_find_devices(ids, 0)) < 0) {
		printf("Error:  Can't find FPGA!\n");
	} else {
		unsigned int p = 0;
		printf("Found FPGA at %02x.%02x.%02x\n",
			PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));

		if (pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &p) || p == 0)
			printf("Error reading FPGA address from PCI config-space\n");
		else
			syscon_base = (volatile unsigned char*)p;
	}

	writel(0x2000, SOC_REGS_PHY_BASE + 0x1816c);
	writel(0x2000, SOC_REGS_PHY_BASE + 0x18170);

	dat = fpga_peek32(0);
	
	printf("fpga_rev=%d\n"
	       "board_id=0x%04X\n",
	       dat & 0xFF,
	       (dat >> 8) & 0xFFFF);

	/* Read ISA_C bank */
	dat = fpga_peek32(0x28);

	/* Set 125MHz OE to 0, which outputs a high signal */
	dat &= ~(0x6);
	fpga_poke32(0x28, dat);

	/* Take phy out of reset */
	fpga_poke32(0x18, 0x2); /* Dat reg */
	dat |= 0x2;
	fpga_poke32(0x28, dat); /* oe */

	/* Allow time for the phys to latch the startup values */
	mdelay(1000);
	/* Enable 125MHz clock for switch */
	dat |= 0x4;
	fpga_poke32(0x28, dat);

	dat = fpga_peek32(4);
	strcpy(tmp_buf, (dat & (1 << 30))?"on":"off");
	env_set("jp_sdboot", tmp_buf);
	strcpy(tmp_buf, (dat & (1 << 31))?"on":"off");
	env_set("jp_uboot", tmp_buf);

#ifndef CONFIG_ENV_IS_IN_SPI_FLASH
	/* Routes CPU pins to FPGA SPI flash.  Happens on normal boots
	* but not when using the SPI daughter card */
	fpga_poke32(0x8, 0xBFFFFFFF);
#endif

	if(i2c_probe(0x54)) {
		printf("Failed to probe silabs at 0x54\n");
	} else {
		uint8_t mac[6];
		i2c_read(0x54, 1536, 2, (uint8_t *)&mac, 6);
		if((mac[0] == 0 && mac[1] == 0 &&
		   mac[2] == 0 && mac[3] == 0 &&
		   mac[4] == 0 && mac[5] == 0) ||
		   (mac[0] == 0xff && mac[1] == 0xff &&
		   mac[2] == 0xff && mac[3] == 0xff &&
		   mac[4] == 0xff && mac[5] == 0xff)) {
			printf("No MAC programmed to board\n");
		} else {
			uchar enetaddr[6];
			enetaddr[5] = mac[0];
			enetaddr[4] = mac[1];
			enetaddr[3] = mac[2];
			enetaddr[2] = mac[3];
			enetaddr[1] = mac[4];
			enetaddr[0] = mac[5];

			eth_env_set_enetaddr("ethaddr", enetaddr);
		}
	}

	hw_watchdog_init();

	__led_set(CONFIG_LED_STATUS_BIT1, 1);

	return 0;
}

uint32_t board_rng_seed(void)
{
	return fpga_peek32(0x44);
}

int checkboard(void)
{
	return 0;
}