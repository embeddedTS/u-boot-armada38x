/*
 * Copyright (C) 2017-2022 Technologic Systems, Inc. dba embeddedTS
 *
 * Author: Mark Featherston <mark@embeddedTS.com.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <status_led.h>

#include "tsfpga.h"

#define TS7840_RIGHT_GRN_LED	(1 << 19)
#define TS7840_RIGHT_RED_LED	(1 << 20)

void __led_init(led_id_t mask, int state)
{
	fpga_dio_oe_set(0, TS7840_RIGHT_GRN_LED |
			   TS7840_RIGHT_RED_LED);
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	u32 bank = fpga_dio_dat_out(0);
#ifdef CONFIG_LED_STATUS0
	if (CONFIG_LED_STATUS_BIT == mask) {
		if(TS7840_RIGHT_GRN_LED & bank)
			__led_set(CONFIG_LED_STATUS_BIT, 1);
		else
			__led_set(CONFIG_LED_STATUS_BIT, 0);
	}
#endif
#ifdef CONFIG_LED_STATUS1
	if (CONFIG_LED_STATUS_BIT1 == mask) {
		if(TS7840_RIGHT_RED_LED & bank)
			__led_set(CONFIG_LED_STATUS_BIT1, 1);
		else
			__led_set(CONFIG_LED_STATUS_BIT1, 0);
	}
#endif
}

void __led_set(led_id_t mask, int state)
{
#ifdef CONFIG_LED_STATUS0
	if (CONFIG_LED_STATUS_BIT == mask) {
		if(state)
			fpga_dio_dat_clr(0, TS7840_RIGHT_GRN_LED);
		else
			fpga_dio_dat_set(0, TS7840_RIGHT_GRN_LED);
	}
#endif

#ifdef CONFIG_LED_STATUS1
	if (CONFIG_LED_STATUS_BIT1 == mask) {
		if(state)
			fpga_dio_dat_clr(0, TS7840_RIGHT_RED_LED);
		else
			fpga_dio_dat_set(0, TS7840_RIGHT_RED_LED);
	}
#endif
}
