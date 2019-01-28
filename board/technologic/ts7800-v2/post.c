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

/* Check for M41T00S */
int m41t00s_rtc_test(void)
{
        int ret;

        ret = i2c_probe(0x68);

        if (ret == 0) printf("RTC test passed\n");
        else printf("RTC test failed\n");
        return ret;
}

int marvell_phy_test(void)
{
	int ret = 0;
	unsigned int oui;
	unsigned char model;
	unsigned char rev;

	if (miiphy_info ("ethernet@70000", 0x1, &oui, &model, &rev) != 0) {
		printf("Failed to find PHY\n");
		return 1;
	}

	if(oui != 0x5043) {
		printf("Wrong PHY?  Bad OUI 0x%X 0x5043\n", oui);
		ret |= 1;
	}

	if(model != 0x1d) {
		printf("Wrong PHY?  Bad model 0x%X not 0x1d\n", oui);
		ret |= 1;
	}

	if (ret == 0) printf("PHY test passed\n");
	else printf("PHY test failed\n");
	return ret;
}

/* Placeholder for now */
int do_post_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	leds_test();

	ret |= m41t00s_rtc_test();
	ret |= marvell_phy_test();

	if (ret == 0) printf("All POST tests passed\n");
	else printf("One or more POST tests failed\n");
	return ret;
}

U_BOOT_CMD(post, 2, 1,	do_post_test,
	"Runs a POST test",
	"[-d]\n"
	"If -d is supplied, the test is destructive to data on eMMC\n"
);
