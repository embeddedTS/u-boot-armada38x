/*
 * watchdog.c - driver for TS-7800/TS-7840 Watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <i2c.h>
#include <watchdog.h>
#include <status_led.h>

#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 30000
#endif

#ifndef CONFIG_SPL_BUILD
static int refcnt = 0;
static int wdtinit = 0;
static ulong lastfeed;

void hw_watchdog_reset(void)
{
	uint8_t val = 0x1;
	if(refcnt || !wdtinit) return;

	/* I2C calls sleep which calls the wdt feeder.  Return immediately
	 * if it is already being called. */
	refcnt = 1;
	/* Our watchdog is over I2C, and u-boot by default feeds the watchdog 
	 * a little too aggressively.  This will rate limit this to no more 
	 * than 1 feed per second */
	if(get_timer(lastfeed) > 1000){
		/*__led_toggle(CONFIG_LED_STATUS_BIT1);*/
		i2c_write(0x54, 1028, 2, &val, 1);
		lastfeed = get_timer(0);
	}
	refcnt = 0;
}

void hw_watchdog_init(void)
{
	uint8_t buf[4];
	/* Specified in 10ms increments */
	const uint32_t timeout = CONFIG_WATCHDOG_TIMEOUT_MSECS / 10;
	i2c_set_bus_num(0);

	buf[0] = timeout;
	buf[1] = timeout >> 8;
	buf[2] = timeout >> 16;
	buf[3] = timeout >> 24;

	i2c_write(0x54, 1024, 2, buf, 4);
	wdtinit = 1;
	lastfeed = get_timer(0);
}

#else
void hw_watchdog_reset(void){};
void hw_watchdog_init(void){};
#endif
