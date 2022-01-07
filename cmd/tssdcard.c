/*
 * (C) Copyright 2018
 * Mark Featherston, mark@embeddedTS.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <tssdcard.h>

static int tssd_curr_dev;

static int do_tssdops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 2) {
		if (strncmp(argv[1], "rescan", 4) == 0) {
			if (tssdcard_get_sectors(tssd_curr_dev, 1) == 0)
				return CMD_RET_FAILURE;
			return CMD_RET_SUCCESS;
		}
	}

	return blk_common_cmd(argc, argv, IF_TYPE_TSSDCARD, &tssd_curr_dev);
}

U_BOOT_CMD(
	tssdcard, 8, 1, do_tssdops,
	"TSSD Driver",
	"rescan - force reenumeration of SD cards\n"
	"tssdcard info - show all available SD cards\n"
	"tssdcard device [dev] - show or set current SD device\n"
	"tssdcard part [dev] - print partition table of one or all SD devices\n"
	"tssdcard read addr blk# cnt - read `cnt' blocks starting at block\n"
	"     `blk#' to memory address `addr'\n"
	"tssdcard write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
);
