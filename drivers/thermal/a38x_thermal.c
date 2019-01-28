// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on armada_thermal.c in Linux 4.4.
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <fuse.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <linux/math64.h>
#include <thermal.h>

#define DFX_CONTROL0_OFF 	0x70
#define DFX_CONTROL1_OFF 	0x74
#define DFX_TEMPSTATUS_OFF 	0x78

#define A38X_COEF_B	1172499100UL
#define A38X_COEF_M	2000096UL
#define A38X_COEF_DIV	4201

#define CONTROL0_TSEN_TC_TRIM_MASK	0x7
#define CONTROL0_TSEN_TC_TRIM_VAL	0x3
#define CONTROL1_EXT_TSEN_HW_RESETn	(1 << 8)
#define CONTROL1_EXT_TSEN_SW_RESET	(1 << 7)

#define STATUS_TEMP_MASK		0x3ff
#define STATUS_TEMP_VALID		(1 << 10)

int read_cpu_temperature(struct udevice *dev)
{
	uint32_t reg;

	do {
		reg = readl(MVEBU_DFX_BASE + DFX_TEMPSTATUS_OFF);
	} while (!(reg & STATUS_TEMP_VALID));
	
	reg &= STATUS_TEMP_MASK;

	return ((A38X_COEF_M * (long)reg) - A38X_COEF_B) / A38X_COEF_DIV;
}

int a38x_thermal_get_temp(struct udevice *dev, int *temp)
{
	int cpu_tmp = 0;

	cpu_tmp = read_cpu_temperature(dev);

	*temp = cpu_tmp;

	return 0;
}

int a38x_thermal_probe(struct udevice *dev)
{
	uint32_t reg;

	/* Disable the HW/SW reset */
	reg = readl(MVEBU_DFX_BASE + DFX_CONTROL1_OFF);
	reg |= CONTROL1_EXT_TSEN_HW_RESETn;
	reg &= ~CONTROL1_EXT_TSEN_SW_RESET;
	writel(reg, MVEBU_DFX_BASE + DFX_CONTROL1_OFF);

	/* Set Tsen Tc Trim to correct default value (errata #132698) */
	reg = readl(MVEBU_DFX_BASE + DFX_CONTROL0_OFF);
	reg &= ~CONTROL0_TSEN_TC_TRIM_MASK;
	reg |= CONTROL0_TSEN_TC_TRIM_VAL;
	writel(reg, MVEBU_DFX_BASE + DFX_CONTROL0_OFF);

	return 0;
}

static int do_cpu_temp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int cpu_temp;
	a38x_thermal_get_temp(NULL, &cpu_temp);
	printf("%d\n", cpu_temp);
	env_set_ulong("cputemp", cpu_temp);
	return 0;
}

U_BOOT_CMD(cputemp, 1, 1,	do_cpu_temp,
	"Print CPU temp in millicelcius",
	"Print CPU temp in millicelcius\n"
);
