/*
 * Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "../drivers/ddr/marvell/a38x/ddr3_a38x_topology.h"
#include <../serdes/a38x/high_speed_env_spec.h>

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

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SGMII2, SERDES_SPEED_1_25_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

/*
 * Define the DDR layout / topology here in the board file. This will
 * be used by the DDR3 init code in the SPL U-Boot version to configure
 * the DDR3 controller.
 */
static struct hws_topology_map ecc_topology_map = {
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    BUS_MASK_32BIT_ECC,		/* memory_width */
	    MEM_4G,			/* mem_size */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    HWS_TEMP_LOW,		/* temperature */
	    HWS_TIM_DEFAULT} },		/* timing */
	5,				/* Num Of Bus Per Interface*/
	BUS_MASK_32BIT_ECC			/* Busses mask */
};

struct hws_topology_map *ddr3_get_topology_map(void)
{
	return &ecc_topology_map;
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x11111111, MVEBU_MPP_BASE + 0x00);
	writel(0x11111111, MVEBU_MPP_BASE + 0x04);
	writel(0x11120001, MVEBU_MPP_BASE + 0x08);
	writel(0x22222201, MVEBU_MPP_BASE + 0x0c);
	writel(0x22220000, MVEBU_MPP_BASE + 0x10);
	writel(0x00000004, MVEBU_MPP_BASE + 0x14);
	writel(0x55000500, MVEBU_MPP_BASE + 0x18);
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c);

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

int checkboard(void)
{
	puts("Board: Technologic TS-7840\n");

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return cpu_eth_init(bis);
}
