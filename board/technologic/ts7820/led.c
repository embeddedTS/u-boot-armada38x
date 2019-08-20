/*
 * Copyright (C) 2017 Technologic Systems
 *
 * Author: Mark Featherston <mark@embeddedarm.com.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <status_led.h>

#include "tsfpga.h"

#define TS78XX_LED_GRN	(1 << 26)
#define TS78XX_LED_RED	(1 << 27)
#define TS78XX_LED_BLU	(1 << 28)

void __led_init(led_id_t mask, int state)
{
	fpga_dio_oe_set(0, TS78XX_LED_RED | TS78XX_LED_GRN | TS78XX_LED_BLU);
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	u32 bank = fpga_dio_dat_out(0);
#ifdef CONFIG_LED_STATUS0
	if (CONFIG_LED_STATUS_BIT == mask) {
		if(TS78XX_LED_GRN & bank)
			__led_set(CONFIG_LED_STATUS_BIT, 1);
		else
			__led_set(CONFIG_LED_STATUS_BIT, 0);
	}
#endif
#ifdef CONFIG_LED_STATUS1
	if (CONFIG_LED_STATUS_BIT1 == mask) {
		if(TS78XX_LED_RED & bank)
			__led_set(CONFIG_LED_STATUS_BIT1, 1);
		else
			__led_set(CONFIG_LED_STATUS_BIT1, 0);
	}
#endif
#ifdef CONFIG_LED_STATUS2
	if (CONFIG_LED_STATUS_BIT2 == mask) {
		if(TS78XX_LED_BLU & bank)
			__led_set(CONFIG_LED_STATUS_BIT2, 0);
		else
			__led_set(CONFIG_LED_STATUS_BIT2, 1);
	}
#endif
}

void __led_set(led_id_t mask, int state)
{
#ifdef CONFIG_LED_STATUS0
	if (CONFIG_LED_STATUS_BIT == mask) {
		if(state)
			fpga_dio_dat_clr(0, TS78XX_LED_GRN);
		else
			fpga_dio_dat_set(0, TS78XX_LED_GRN);
	}
#endif

#ifdef CONFIG_LED_STATUS1
	if (CONFIG_LED_STATUS_BIT1 == mask) {
		if(state)
			fpga_dio_dat_clr(0, TS78XX_LED_RED);
		else
			fpga_dio_dat_set(0, TS78XX_LED_RED);
	}
#endif

#ifdef CONFIG_LED_STATUS2
	if (CONFIG_LED_STATUS_BIT2 == mask) {
		if(state)
			fpga_dio_dat_set(0, TS78XX_LED_BLU);
		else
			fpga_dio_dat_clr(0, TS78XX_LED_BLU);
	}
#endif
}
