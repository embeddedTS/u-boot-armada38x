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

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	uint32_t val;
#ifdef CONFIG_LED_STATUS_BIT
	if(CONFIG_LED_STATUS_BIT & mask) {
		val = fpga_peek32(0x8);
		fpga_poke32(0x8, val ^ (1 << 30));
	}
#endif
#ifdef CONFIG_LED_STATUS_BIT1
	if(CONFIG_LED_STATUS_BIT1 & mask) {
		val = fpga_peek32(0xc);
		fpga_poke32(0xc, val ^ (1 << 20));
	}
#endif
}

void __led_set(led_id_t mask, int state)
{
	uint32_t val;
#ifdef CONFIG_LED_STATUS_BIT
	if (CONFIG_LED_STATUS_BIT & mask) {
		if(state) {
			val = fpga_peek32(0x8);
			fpga_poke32(0x8, val | (1 << 30));
		} else {
			val = fpga_peek32(0x8);
			fpga_poke32(0x8, val & ~(1 << 30));
		}
	}
#endif
#ifdef CONFIG_LED_STATUS_BIT1
	if (CONFIG_LED_STATUS_BIT1 & mask) {
		if(state) {
			val = fpga_peek32(0xc);
			fpga_poke32(0xc, val | (1 << 20));
		} else {
			val = fpga_peek32(0xc);
			fpga_poke32(0xc, val & ~(1 << 20));
		}
	}
#endif
}

#ifdef CONFIG_LED_STATUS_GREEN
void green_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, 0);
}

void green_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_GREEN, 1);
}
#endif

#ifdef CONFIG_LED_STATUS_RED
void red_led_off(void)
{
	__led_set(CONFIG_LED_STATUS_RED, 0);
}

void red_led_on(void)
{
	__led_set(CONFIG_LED_STATUS_RED, 1);
}
#endif