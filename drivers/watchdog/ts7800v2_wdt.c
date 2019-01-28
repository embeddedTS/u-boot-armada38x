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
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 60000
#endif

#ifndef CONFIG_SPL_BUILD
static int refcnt = 0;
static int wdtinit = 0;
static ulong lastfeed;

extern uint8_t get_bootflags(void);

void reset_misc(void)
{
	/* timeout 0 == watchdog off.  1 is 10ms,
	 * the smallest amount we can arm for */
	uint32_t timeout = 1;
	uint8_t val = 0x1;

	i2c_write(0x54, 1024, 2, (uint8_t *)&timeout, 4);
	i2c_write(0x54, 1028, 2, &val, 1);

	while(1);
}

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
	/* Specified in 10ms increments */
	uint32_t timeout = CONFIG_WATCHDOG_TIMEOUT_MSECS / 10;
	i2c_set_bus_num(0);

	/* If watchdog is not enabled in boot flags, do not feed watchdog */
	if(get_bootflags() & 0x1){
		i2c_write(0x54, 1024, 2, (uint8_t *)&timeout, 4);
		wdtinit = 1;
		lastfeed = get_timer(0);
	}
}

#else
void hw_watchdog_reset(void){};
void hw_watchdog_init(void){};
#endif
