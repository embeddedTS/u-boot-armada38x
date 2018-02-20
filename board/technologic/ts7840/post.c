/*
 * Copyright (C) 2017 Technologic Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/gpio.h>
#include <cli.h>
#include <command.h>
#include <i2c.h>
#include <spi.h>
#include <status_led.h>
#include <usb.h>

#include <miiphy.h>

void leds_test(void)
{
	int i;

	__led_set(CONFIG_LED_STATUS_BIT | CONFIG_LED_STATUS_BIT1,
			  CONFIG_LED_STATUS_ON);

	for(i = 0; i < 24; i++){
		if(i % 4 == 0) __led_set(CONFIG_LED_STATUS_BIT1, CONFIG_LED_STATUS_ON);
		else __led_set(CONFIG_LED_STATUS_BIT1, CONFIG_LED_STATUS_OFF);
		if(i % 4 == 1) __led_set(CONFIG_LED_STATUS_BIT, CONFIG_LED_STATUS_ON);
		else __led_set(CONFIG_LED_STATUS_BIT, CONFIG_LED_STATUS_OFF);
		mdelay(100);
	}

	__led_set(CONFIG_LED_STATUS_BIT | CONFIG_LED_STATUS_BIT1,
			  CONFIG_LED_STATUS_ON);
}

/* Placeholder for now */
static int do_post_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	leds_test();

	if (ret == 0) printf("All POST tests passed\n");
	else printf("One or more POST tests failed\n");
	return ret;
}

U_BOOT_CMD(post, 2, 1,	do_post_test,
	"Runs a POST test",
	"[-d]\n"
	"If -d is supplied, the test is destructive to data on eMMC\n"
);
