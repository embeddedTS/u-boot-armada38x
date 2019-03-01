/*
 * Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <watchdog.h>

#include "../drivers/ddr/mv-ddr-marvell/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

#include "tsfpga.h"

DECLARE_GLOBAL_DATA_PTR;

#define ETH_PHY_CTRL_REG		0
#define ETH_PHY_CTRL_POWER_DOWN_BIT	11
#define ETH_PHY_CTRL_POWER_DOWN_MASK	(1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

#define BOOTFLAG_REG		1543
#define BOOTFLAG_EN_WDOG	0x1

void __iomem *syscon_base;

static struct serdes_map board_serdes_map[] = {
	{SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
};

int get_board_model(void)
{
	static int model = 0;
	if(model == 0)
	{
		if(of_machine_is_compatible("technologic,a385-ts7840"))
			model = 0x7840;
		else if (of_machine_is_compatible("technologic,a385-ts7820"))
			model = 0x7820;
	}
	return model;
}

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

static struct mv_ddr_topology_map ts7820_ecc_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
		{0x1, 0, 0, 0},
		{0x1, 0, 0, 0},
		{0x1, 0, 0, 0},
		{0x1, 0, 0, 0} },
		SPEED_BIN_DDR_1600K,   /* speed_bin */
		MV_DDR_DEV_WIDTH_32BIT,   /* sdram device width */
		MV_DDR_DIE_CAP_4GBIT,  /* die capacity */
		DDR_FREQ_800,    /* frequency */
		0, 0,         /* cas_l cas_wl */
		MV_DDR_TEMP_LOW} },    /* temperature */
	BUS_MASK_32BIT_ECC,     /* subphys mask */
	MV_DDR_CFG_DEFAULT,     /* ddr configuration data source */
	{ {0} },       /* raw spd data */
	{0}            /* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	/* Return the board topology as defined in the board code */
	return &ts7820_ecc_topology_map;
}

uint8_t get_bootflags(void)
{
   static uint8_t runonce = 0;
   static uint8_t bootflags = 0;
   if(runonce != 1) {
      i2c_read(0x54, BOOTFLAG_REG, 2, &bootflags, 1);
      runonce = 1;
   }
   return bootflags;
}

void board_spi_cs_activate(int cs)
{
	if(cs == 1) {
		/* Route CS to onboard flash */
		writel(0x20000, 0xf1018170);
		fpga_dio_dat_clr(0, (1 << 12) | (1 << 13));
	} else if(cs == 2){
		/* Route CS to offboard flash */
		writel(0x20000, 0xf1018174);
		fpga_dio_dat_clr(0, (1 << 12) | (1 << 13));
	}
}

void board_spi_cs_deactivate(int cs)
{
	/* Same CS is going to cs 1/2, but activate picks the mux */
	if(cs == 1 || cs == 2) {
		fpga_dio_dat_set(0, (1 << 12) | (1 << 13));
	}
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00); /* 7:0 */
	writel(0x11111111, MVEBU_MPP_BASE + 0x04); /* 15:8 */
	writel(0x11200011, MVEBU_MPP_BASE + 0x08); /* 23:16 */
	writel(0x22222011, MVEBU_MPP_BASE + 0x0c); /* 31:24 */
	writel(0x22200002, MVEBU_MPP_BASE + 0x10); /* 39:32 */
	writel(0x00000022, MVEBU_MPP_BASE + 0x14); /* 47:40 */
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

void inc_mac(uchar *enetaddr)
{
	if (enetaddr[5] == 0xff) {
		if (enetaddr[4] == 0xff) {
			enetaddr[3]++;
		}
		enetaddr[4]++;
	}
	enetaddr[5]++;
}

int board_late_init(void)
{
	int ret;
	pci_dev_t dev;
	uint8_t tmp[6];
	uchar enetaddr[6];
	unsigned int p = 0;

	struct pci_device_id ids[2] = {
		{TS7820_FPGA_VENDOR, TS7820_FPGA_DEVICE},
		{0, 0}
	};

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	switch(get_board_model()){
	case 0x7820:
		env_set("board_revision", "P1");
		env_set("board_name", "TS-7820");
		env_set("board_model", "7820");
		break;
	case 0x7840:
		env_set("board_revision", "P2");
		env_set("board_name", "TS-7840");
		env_set("board_model", "7840");
		break;
	default:
		puts("Unknown board\n");
	}
#endif

	dev = pci_find_devices(ids, 0);
	if(!dev){
		printf("Error: Can't find FPGA!\n");
		return 1;
	}

	if (pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &p) || p == 0)
		printf("Error reading FPGA address from PCI config-space\n");
	else
		syscon_base = (unsigned char*)p;

	printf("FPGA Revision: %d\n", readl(syscon_base + 0x4) & 0xffff);

	/* When CLK_125MHz OE is low, drives high */
	fpga_dio_oe_clr(0, (1 << 3));

	/* Enable PHY */
	fpga_dio_dat_clr(0, (1 << 18));
	fpga_dio_oe_set(0, (1 << 18));

	/* 88E1512 requires 10ms minimum */
	udelay(10*1000);
	fpga_dio_dat_set(0, (1 << 18));

	/* 88E1512 requires time to strap phy addr */
	udelay(1000);
	/* Enable switch 125MHz clock */
	fpga_dio_oe_set(0, (1 << 3));

	ret = i2c_probe(0x54);
	if(ret) { 
		printf("Failed to probe silabs at 0x54\n");
		return 1;
	}
	i2c_read(0x54, 1536, 2, (uint8_t *)&tmp, 6);
	enetaddr[5] = tmp[0];
	enetaddr[4] = tmp[1];
	enetaddr[3] = tmp[2];
	enetaddr[2] = tmp[3];
	enetaddr[1] = tmp[4];
	enetaddr[0] = tmp[5];

	if (!is_valid_ethaddr(enetaddr)) {
		printf("No MAC programmed to board\n");
	} else {
		/* TS MAC addresses are always assigned sequentially */
		eth_env_set_enetaddr("eth1addr", enetaddr);
		inc_mac(enetaddr);
		eth_env_set_enetaddr("eth2addr", enetaddr);
		inc_mac(enetaddr);
		eth_env_set_enetaddr("eth3addr", enetaddr);
	}

	hw_watchdog_init();

	/* FPGA_FLASH_SELECT DDR output */
	writel(0x20000, 0xf101816c);

	/* Enable USB 5V, EN_SD_3V3 */
	fpga_dio_dat_set(0, (1 << 16) | (1 << 3));

	/* Enable RED led only to indicate boot progress */
	__led_set(CONFIG_LED_STATUS_BIT, 0); /* Green */
	__led_set(CONFIG_LED_STATUS_BIT1, 1); /* Red */
	__led_set(CONFIG_LED_STATUS_BIT2, 0); /* Blue */

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
