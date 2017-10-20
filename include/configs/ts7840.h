/*
 * Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_TS7840_H
#define _CONFIG_TS7840_H

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_DISPLAY_BOARDINFO_LATE

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */
#define	CONFIG_SYS_TEXT_BASE		0x00800000
#define CONFIG_SYS_TCLK			250000000	/* 200MHz */

#define CONFIG_NR_DRAM_BANKS		1

/*
 * Commands configuration
 */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_SF_DEFAULT_SPEED		1000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#define CONFIG_SPI_FLASH_STMICRO

/*
 * SDIO/MMC Card Configuration
 */
#define CONFIG_SYS_MMC_BASE		MVEBU_SDIO_BASE
#define CONFIG_SUPPORT_EMMC_BOOT

/* Partition support */

/* Additional FS support/configuration */
#define CONFIG_SUPPORT_VFAT

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

#define CONFIG_ENV_MIN_ENTRIES		128

/* Env is at the 1MB boundary in emmc boot partition 0 */
#define CONFIG_SYS_MMC_ENV_DEV  	0 /* mmcblk0 */
#define CONFIG_SYS_MMC_ENV_PART 	1 /* boot0 */
#define CONFIG_ENV_OFFSET		0x400000 /* 8MiB */
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_OFFSET_REDUND 	0x800000 /* 12MiB */
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_MVEBU
#define CONFIG_PCI_SCAN_SHOW
#endif

#define CONFIG_SYS_ALT_MEMTEST

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define RELOCATION_LIMITS_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 * SPL_BOOT_SDIO_MMC_CARD	- Booting via SDIO/MMC card (partition 1)
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define SPL_BOOT_SDIO_MMC_CARD		2
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SDIO_MMC_CARD

#define CONFIG_NET_RANDOM_ETHADDR

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_TEXT_BASE		0x40000030
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x0030)
/*#define CONFIG_SPL_MAX_SIZE 	(1024*500)*/

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
/* SPL related SPI defines */
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SDIO_MMC_CARD
/* SPL related MMC defines */
#define CONFIG_SYS_MMC_U_BOOT_OFFS		(160 << 10)
#define CONFIG_SYS_U_BOOT_OFFS			CONFIG_SYS_MMC_U_BOOT_OFFS
#ifdef CONFIG_SPL_BUILD
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER	0x00180000	/* in SDRAM */
#endif
#endif

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Include the common distro boot environment */
#ifndef CONFIG_SPL_BUILD

#define CONFIG_NFS_TIMEOUT 100UL

/*#define CONFIG_MISC_INIT_R*/

#define KERNEL_ADDR_R	__stringify(0x800000)
#define FDT_ADDR_R	__stringify(0x100000)
#define RAMDISK_ADDR_R	__stringify(0x1800000)
#define SCRIPT_ADDR_R	__stringify(0x200000)
#define PXEFILE_ADDR_R	__stringify(0x300000)

#define LOAD_ADDRESS_ENV_SETTINGS \
	"loadaddr=" KERNEL_ADDR_R "\0" \
	"fdtaddr=" FDT_ADDR_R "\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	LOAD_ADDRESS_ENV_SETTINGS \
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0" \
	"model=7840\0" \
	"nfsroot=192.168.0.36:/mnt/storage/a38x\0" \
	"autoload=no\0" \
	"ethact=ethernet@70000\0" \
	"cmdline_append=console=ttyS0,115200 init=/sbin/init\0" \
	"clearenv=mmc dev 0 1; mmc erase 2000 2000; mmc erase 4000 2000;\0" \
	"usbprod=usb start;" \
		"if usb storage;" \
			"then echo Checking USB storage for updates;" \
			"if load usb 0:1 ${loadaddr} /tsinit.ub;" \
				"then led green on;" \
				"source ${loadaddr};" \
				"led red off;" \
				"exit;" \
			"fi;" \
		"fi;\0" \
	"emmcboot=echo Booting from the eMMC ...;" \
		"if load mmc 0:1 ${loadaddr} /boot/boot.ub;" \
			"then echo Booting from custom /boot/boot.ub;" \
			"source ${loadaddr};" \
		"fi;" \
		"if load mmc 0:1 ${loadaddr} /boot/ts7970-fpga.vme;" \
			"then fpga load 0 ${loadaddr} ${filesize};" \
		"fi;" \
		"load mmc 0:1 ${fdtaddr} /boot/imx6${cpu}-ts7970.dtb;" \
		"load mmc 0:1 ${loadaddr} /boot/uImage;" \
		"setenv bootargs root=/dev/mmcblk0p1 ${cmdline_append};" \
		"bootm ${loadaddr} - ${fdtaddr};\0" \
	"sataboot=echo Booting from SATA ...;" \
		"sata init;" \
		"if load sata 0:1 ${loadaddr} /boot/boot.ub;" \
			"then echo Booting from custom /boot/boot.ub;" \
			"source ${loadaddr};" \
		"fi;" \
		"if load sata 0:1 ${loadaddr} /boot/ts7970-fpga.vme;" \
			"then fpga load 0 ${loadaddr} ${filesize};" \
		"fi;" \
		"load sata 0:1 ${fdtaddr} /boot/imx6${cpu}-ts7970.dtb;" \
		"load sata 0:1 ${loadaddr} /boot/uImage;" \
		"setenv bootargs root=/dev/sda1 rootwait ${cmdline_append};" \
		"bootm ${loadaddr} - ${fdtaddr};\0" \
	"usbprod=usb start;" \
		"if usb storage;" \
			"then echo Checking USB storage for updates;" \
			"if load usb 0:1 ${loadaddr} /tsinit.ub;" \
				"then led green on;" \
				"source ${loadaddr};" \
				"led red off;" \
				"exit;" \
			"fi;" \
		"fi;\0" \
	"nfsboot=echo Booting from NFS ...;" \
		"dhcp;" \
		"nfs ${fdtaddr} ${nfsroot}/boot/armada-385-${MODEL}.dtb;" \
		"nfs ${loadaddr} ${nfsroot}/boot/zImage;" \
		"setenv bootargs root=/dev/nfs ip=dhcp nfsroot=${nfsroot} " \
			"${cmdline_append};" \
		"bootz ${loadaddr} - ${fdtaddr}; \0"

#define CONFIG_BOOTCOMMAND \
	"run nfsboot;"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_VERSION_VARIABLE
#define CONFIG_SYS_CBSIZE	       1024

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	       32
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#endif /* CONFIG_SPL_BUILD */

#endif /* _CONFIG_TS7840_H */
