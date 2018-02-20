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
#define BOOTFLAG_EN_ECC		0x40
#define BOOTFLAG_EN_MPCIE	0x80

static struct serdes_map board_serdes_map[] = {
   {SATA0, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
   {USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
   {SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
   {USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
   {PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0}, /* Not connected */
   {PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
};

volatile unsigned char *syscon_base;

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

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
   if(get_bootflags() & BOOTFLAG_EN_MPCIE) {
      board_serdes_map[0].serdes_type = PEX0;
      board_serdes_map[0].serdes_speed = SERDES_SPEED_5_GBPS;
      board_serdes_map[0].serdes_mode = PEX_ROOT_COMPLEX_X1;
      board_serdes_map[0].swap_rx = 1;
   }

   *serdes_map_array = board_serdes_map;
   *count = ARRAY_SIZE(board_serdes_map);
   return 0;
}

static struct mv_ddr_topology_map ts7800_noecc_topology_map = {
   DEBUG_LEVEL_ERROR,
   0x1, /* active interfaces */
   /* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
   { { { {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0} },
       SPEED_BIN_DDR_1600K,	/* speed_bin */
       MV_DDR_DEV_WIDTH_16BIT,	/* sdram device width */
       MV_DDR_DIE_CAP_4GBIT,	/* die capacity */
       DDR_FREQ_800,		/* frequency */
       0, 0,			/* cas_l cas_wl */
       MV_DDR_TEMP_LOW} },		/* temperature */
   BUS_MASK_32BIT,			/* subphys mask */
   MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
   { {0} },			/* raw spd data */
   {0}				/* timing parameters */
};

/* Disables 32-bit DDR interface and uses 16-bit.  We normally have 1GB, but
 * this lets us have 512MB and use the additional chip for ECC */
static struct mv_ddr_topology_map ts7800_ecc_topology_map = {
   DEBUG_LEVEL_INFO,
   0x1, /* active interfaces */
   /* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
   { { { {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0},
         {0x1, 0, 0, 0} },
       SPEED_BIN_DDR_1600K,	/* speed_bin */
       MV_DDR_DEV_WIDTH_16BIT,	/* sdram device width */
       MV_DDR_DIE_CAP_4GBIT,	/* die capacity */
       DDR_FREQ_800,		/* frequency */
       0, 0,			/* cas_l cas_wl */
       MV_DDR_TEMP_LOW} },		/* temperature */
   BUS_MASK_16BIT_ECC_PUP3,	/* subphys mask */
   MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
   { {0} },			/* raw spd data */
   {0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
   /*if(get_bootflags() & BOOTFLAG_EN_ECC) {
      return &ts7800_ecc_topology_map;
   }*/
   return &ts7800_noecc_topology_map;
}

void board_spi_cs_activate(int cs)
{
   if(cs == 1) {
      writel(0x10000000, SOC_REGS_PHY_BASE + 0x18134);
   }
}

void board_spi_cs_deactivate(int cs)
{
   if(cs == 1) {
      writel(0x10000000, SOC_REGS_PHY_BASE + 0x18130);
   }
}

int board_early_init_f(void)
{
   /**
   0x11111111
      MPP[0],  UART0_RXD, 1
      MPP[1],  UART0_TXD, 1
      MPP[2],  I2C_0_CLK, 1
      MPP[3],  I2C_0_DAT, 1
      MPP[4],  MPP4_GE_MDC, 1
      MPP[5],  MPP5_GE_MDIO, 1
      MPP[6],  MPP6_GE_TXCLK, 1
      MPP[7],  MPP7_GE_TXD0, 1
   0x11111111
      MPP[8],  MPP8_GE_TXD1, 1
      MPP[9],  MPP9_GE_TXD2, 1
      MPP[10], MPP10_GE_TXD3, 1
      MPP[11], MPP11_GE_TXCTL, 1
      MPP[12], MPP12_GE_RXD0, 1
      MPP[13], MPP13_GE_RXD1, 1
      MPP[14], MPP14_GE_RXD2, 1
      MPP[15], MPP15_GE_RXD3, 1
   0x11000011
      MPP[16], MPP16_GE_RXCTL, 1
      MPP[17], MPP17_GE_RXCLK, 1
      MPP[18], WIFI_IRQ#, 0
      MPP[19], CPU_IRQ, 0
      MPP[20], NC
      MPP[21], NC
      MPP[22], SPI_0_MOSI, 1
      MPP[23], SPI_0_CLK, 1
   0x00001011
      MPP[24], SPI_0_MISO, 1
      MPP[25], SPI_0_BOOT_CS0#, 1
      MPP[26], NC
      MPP[27], SPI_0_WIFI_CS2#, 1
      MPP[28], SPI_0_CS3#, 0 (SPI CS not an available function on this pin, so use GPIO)
      MPP[29], GE_PHY_INT#, 0
      MPP[30], CPU_SPEED_1, 0
      MPP[31], CPU_SPEED_2, 0
   0x00000000
      MPP[32], NC
      MPP[33], CPU_SPEED_0, 0
      MPP[34], CPU_SPEED_3, 0
      MPP[35], CPU_SPEED_4, 0
      MPP[36], CPU_TYPE_0, 0
      MPP[37], NC
      MPP[38], NC
      MPP[39], NC
   0x00000000
      MPP[40], NC
      MPP[41], NC
      MPP[42], EN_MMC_PWR, 0
      MPP[43], EN_FAN, 0
      MPP[44], CPU_TYPE_1, 0
      MPP[45], EN_USB_HOST_5V, 0
      MPP[46], FPGA_FLASH_SELECT, 0
      MPP[47], NC
   0x55010500
      MPP[48], NC
      MPP[49], ACCEL_2_INT, 0
      MPP[50], EMMC_CMD, 5
      MPP[51], SPREAD_SPECTRUM#, 0
      MPP[52], DETECT_MSATA, 1
      MPP[53], DETECT_9478, 0
      MPP[54], EMMC_D3, 5
      MPP[55], EMMC_D0, 5
   0x00005550
      MPP[56], NC
      MPP[57], EMMC_CLK, 5
      MPP[58], EMMC_D1, 5
      MPP[59], EMMC_D2, 5

   */
   /* Configure MPP */
   writel(0x11111111, MVEBU_MPP_BASE + 0x00); /* 7:0 */
   writel(0x11111111, MVEBU_MPP_BASE + 0x04); /* 15:8 */
   writel(0x11000011, MVEBU_MPP_BASE + 0x08); /* 23:16 */
   writel(0x00001011, MVEBU_MPP_BASE + 0x0c); /* 31:24 */
   writel(0x00000000, MVEBU_MPP_BASE + 0x10); /* 39:32 */
   writel(0x00000000, MVEBU_MPP_BASE + 0x14); /* 47:40 */
   writel(0x55010500, MVEBU_MPP_BASE + 0x18); /* 55:48 */
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

#if (0)
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
#endif

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
   int ret;
   char tmp_buf[10];
   unsigned int syscon_reg;
   uint8_t buf;
   pci_dev_t dev;

   struct pci_device_id ids[2] = {
      {TS7800_V2_FPGA_VENDOR, TS7800_V2_FPGA_DEVICE},
      {0, 0}
   };

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
   env_set("board_revision", "P1");
   env_set("board_name", "TS-7800-V2");
   env_set("board_model", "7800-V2");
#endif

   writel(0, SOC_REGS_PHY_BASE + 0xa381c);   /* Test Configuration Reg in RTC */

   /* default, in case  we can't find it by a PCI scan */
   syscon_base = TS7800_V2_SYSCON_BASE;

   if ((dev = pci_find_devices(ids, 0)) < 0)
      printf("Error:  Can't find FPGA!\n");
   else {
      unsigned int p = 0;
      printf("Found FPGA at %02x.%02x.%02x\n",
                   PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));

      if (pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &p) || p == 0)
         printf("Error reading FPGA address from PCI config-space\n");
      else
         syscon_base = (volatile unsigned char*)p;
   }

   syscon_reg = fpga_peek32(0);

   printf("fpga_rev=0x%02X\n"
          "board_id=0x%04X\n",
          syscon_reg & 0xFF,
          (syscon_reg >> 8) & 0xFFFF);

   buf = get_bootflags();
   puts("Bootflags: WDOG");

   if(buf & BOOTFLAG_EN_WDOG) putc('+');
   else putc('-');

   puts(" ECC");
   if(buf & BOOTFLAG_EN_ECC) putc('+');
   else putc('-');

   if(buf & BOOTFLAG_EN_MPCIE) puts(" PCIE+ MSATA-\n");
   else puts(" PCIE- MSATA+\n");

   snprintf(tmp_buf, sizeof(tmp_buf),
      "0x%02X", syscon_reg & 0xFF);
   env_set("fpga_rev", tmp_buf);
   snprintf(tmp_buf, sizeof(tmp_buf),
      "0x%04X", (syscon_reg >> 8) & 0xFFFF);
   env_set("board_id", tmp_buf);

   syscon_reg = fpga_peek32(4);

   strcpy(tmp_buf, (syscon_reg & (1 << 30))?"on":"off");
   env_set("jp_sdboot", tmp_buf);

   strcpy(tmp_buf, (syscon_reg & (1 << 31))?"on":"off");
   env_set("jp_uboot", tmp_buf);

   /* Enable EN_USB_5V */
   writel(0x2000, SOC_REGS_PHY_BASE + 0x1816c);
   writel(0x2000, SOC_REGS_PHY_BASE + 0x18170);

#ifndef CONFIG_ENV_IS_IN_SPI_FLASH
   /* Routes CPU pins to FPGA SPI flash.  Happens on normal boots
    * but not when using the SPI daughter card */
   fpga_poke32(0x8, 0xBFFFFFFF);
#endif

   ret = i2c_probe(0x54);
   if(ret) {
      printf("Failed to probe silabs at 0x54\n");
   } else {
      uint8_t mac[6];
      i2c_read(0x54, 1536, 2, (uint8_t *)&mac, 6);

      if (!is_valid_ethaddr(mac))      {
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
