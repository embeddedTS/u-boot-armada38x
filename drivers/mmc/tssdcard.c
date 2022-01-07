/*
 * Qualcomm SDHCI driver - SD/eMMC controller
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mark@embeddedTS.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <fdtdec.h>
#include <part.h>
#include <os.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <dm/device-internal.h>
#include <asm/io.h>

#define CONFIG_TSSDCARD_CORES_MAX 1

#define SDPEEK8(sd, x) readb((uint32_t *)((sd)->sd_regstart + (x)))
#define SDPOKE8(sd, x, y) writeb(y, (uint32_t *)((sd)->sd_regstart + (x)))
#define SDPEEK16(sd, x) readw((uint32_t *)((sd)->sd_regstart + (x)))
#define SDPOKE16(sd, x, y) writew(y, (uint32_t *)((sd)->sd_regstart + (x)))
#define SDPEEK32(sd, x) readl((uint32_t *)((sd)->sd_regstart + (x)))
#define SDPOKE32(sd, x, y) writel(y, (uint32_t *)((sd)->sd_regstart + (x)))

/* Disable probing for eMMC, we only connect this core here to SD */
#define SD_NOMMC
#define SD_NOAUTOMMC
#define SDCORE_NDEBUG

/* Layer includes low level hardware support */
#include "tssdcore2.c"

struct tssdcard_plat {
	struct sdcore tssdcore;
	struct blk_desc blk_dev;
	unsigned long timeout;
	int probed;
};

DECLARE_GLOBAL_DATA_PTR;

static struct tssdcard_plat tssdcard[CONFIG_TSSDCARD_CORES_MAX];

static struct tssdcard_plat *find_tssdcard(int dev)
{
	if (dev >= 0 && dev < CONFIG_TSSDCARD_CORES_MAX)
		return &tssdcard[dev];

	return NULL;
}

static void tssdcard_reset_timeout(void *data)
{
	struct sdcore *sd = (struct sdcore *)data;
	struct tssdcard_plat *plat = (struct tssdcard_plat *)sd->os_arg;

	/* SD Spec allows 1 second for cards to answer */
	plat->timeout = get_timer(0);
}

static int tssdcard_timeout(void *data)
{
	struct sdcore *sd = (struct sdcore *)data;
	struct tssdcard_plat *plat = (struct tssdcard_plat *)sd->os_arg;

	return get_timer(plat->timeout) > CONFIG_SYS_HZ;
}

uint32_t tssdcard_get_sectors(int dev, int force_rescan)
{
	struct tssdcard_plat *plat = find_tssdcard(dev);

	if(!plat)
		return 0;

	if(force_rescan || plat->blk_dev.lba == 0)
		plat->blk_dev.lba = sdreset(&plat->tssdcore);

	return sdreset(&plat->tssdcore);
}

static unsigned long tssdcard_block_read(struct blk_desc *block_dev,
				     unsigned long start, lbaint_t blkcnt,
				     void *buffer)
{
	struct tssdcard_plat *plat = find_tssdcard(block_dev->devnum);

	if (!plat)
		return -1;

	if(!plat->blk_dev.lba)
		return -1;

	if (plat->blk_dev.lba < start + blkcnt) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}

	tssdcard_reset_timeout(&plat->tssdcore);
	if(sdread(&plat->tssdcore, start, buffer, blkcnt) != 0) {
		printf("Error, can't access SD card\n");
		return -1;
	}

	return blkcnt;
}

static unsigned long tssdcard_block_write(struct blk_desc *block_dev,
				      unsigned long start, lbaint_t blkcnt,
				      const void *buffer)
{
	struct tssdcard_plat *plat = find_tssdcard(block_dev->devnum);

	if (!plat)
		return -1;

	if(!plat->blk_dev.lba)
		return -1;

	if (plat->blk_dev.lba < start + blkcnt) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}

	if(!plat->tssdcore.sd_wprot) {
		printf("Card is write protected!\n");
		return -1;
	}

	tssdcard_reset_timeout(&plat->tssdcore);
	if(sdwrite(&plat->tssdcore, start, (unsigned char *)buffer, blkcnt) != 0) {
		printf("Error, can't access SD card\n");
		return -1;
	}

	return blkcnt;
}

static void tssdcard_irqwait(void *data, unsigned int x)
{
	struct tssdcard_plat *plat = (struct tssdcard_plat *)data;
	uint32_t reg;

	do {
#ifdef CONFIG_PREEMPT_NONE
		/* Default marvell kernel config has no preempt, so
		 * to support that:
		 */
		cond_resched();
#endif
		reg = readl((uint32_t *)(plat->tssdcore.sd_regstart + 0x108));
	} while ((reg & 4) == 0);
}

static void tssdcard_delay(void *data, unsigned int us)
{
	udelay(us);
}

/* For now support 1 lun, update this when we add more */
int tssdcard_init(int luns, unsigned char *base)
{
	struct tssdcard_plat *plat = find_tssdcard(luns);

	if (!plat)
		return -1;

	plat->tssdcore.sd_regstart = (unsigned int)base;
	plat->tssdcore.sd_lun = luns;
	plat->tssdcore.os_timeout = tssdcard_timeout;
	plat->tssdcore.os_reset_timeout = tssdcard_reset_timeout;
	plat->tssdcore.os_arg = plat;
	plat->tssdcore.os_delay = tssdcard_delay;
	plat->tssdcore.os_irqwait = tssdcard_irqwait;
	plat->tssdcore.sd_writeparking = 1;
	plat->blk_dev.lba = 0;
	plat->probed = 0;

	return 0;
}

void tssdcard_probe(int lun)
{
	struct tssdcard_plat *plat = find_tssdcard(lun);
	struct blk_desc *blk_dev;

	if(!plat)
		return;

	if(plat->probed) return;

	blk_dev = &plat->blk_dev;
	blk_dev->if_type = IF_TYPE_TSSDCARD;
	blk_dev->priv = plat;
	blk_dev->blksz = 512;
	blk_dev->lba = sdreset(&plat->tssdcore);
	blk_dev->block_read = tssdcard_block_read;
	blk_dev->block_write = tssdcard_block_write;
	blk_dev->devnum = lun;
	blk_dev->part_type = PART_TYPE_UNKNOWN;

	part_init(blk_dev);
	plat->probed = 1;
}

int tssdcard_get_dev(int devnum, struct blk_desc **blk_devp)
{
	struct tssdcard_plat *plat = find_tssdcard(devnum);

	if (!plat){
		return -ENODEV;
	}

	if(plat->blk_dev.lba == 0) {
		plat->blk_dev.lba = sdreset(&plat->tssdcore);
		if(plat->blk_dev.lba != 0)
			tssdcard_probe(devnum);
	}

	*blk_devp = &plat->blk_dev;

	return 0;
}

U_BOOT_LEGACY_BLK(tssdcard) = {
	.if_typename	= "tssdcard",
	.if_type	= IF_TYPE_TSSDCARD,
	.max_devs	= CONFIG_TSSDCARD_CORES_MAX,
	.get_dev	= tssdcard_get_dev,
};
