/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web_http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#ifndef _INC_A38X_TOPOLOGY_H
#define _INC_A38X_TOPOLOGY_H

#include "mvDdr3LoggingDef.h"

/*Bus mask variants*/
#define INTERFACE_BUS_MASK_32BIT				0xF
#define INTERFACE_BUS_MASK_32BIT_ECC			0x1F
#define INTERFACE_BUS_MASK_16BIT				0x3
#define INTERFACE_BUS_MASK_16BIT_ECC			0x13
#define INTERFACE_BUS_MASK_16BIT_ECC_PUP3		0xB

#define  DYNAMIC_CS_SIZE_CONFIG
#define  DISABLE_L2_FILTERING_DURING_DDR_TRAINING

#define SPEED_BIN_DDR_CLEARFOG_BOARD	SPEED_BIN_DDR_1600K
#define BUS_WIDTH_CLEARFOG_BOARD	BUS_WIDTH_16

#ifdef CONFIG_DDR3
	#define SPEED_BIN_DDR_CUST_BOARD_1	SPEED_BIN_DDR_1866L
	#define BUS_WIDTH_CUST_BOARD_1		BUS_WIDTH_8
	#define SPEED_BIN_DDR_CUST_BOARD_2	SPEED_BIN_DDR_1866L
	#define BUS_WIDTH_CUST_BOARD_2		BUS_WIDTH_8

	#define SPEED_BIN_DDR_DB_68XX		SPEED_BIN_DDR_1866L
	#define BUS_WIDTH_DB_68XX			BUS_WIDTH_8
	#define SPEED_BIN_DDR_DB_BP_6821	SPEED_BIN_DDR_1866L
	#define BUS_WIDTH_DB_BP_6821		BUS_WIDTH_8
#else
/*Elpida by 16 is 2400R Micron by 8 is 2133P Samsung by 16 is 2133P Hynux by16 is 2133P*/
	#define SPEED_BIN_DDR_CUST_BOARD_1	SPEED_BIN_DDR_2400R
	#define BUS_WIDTH_CUST_BOARD_1		BUS_WIDTH_16
	#define SPEED_BIN_DDR_CUST_BOARD_2	SPEED_BIN_DDR_2400R
	#define BUS_WIDTH_CUST_BOARD_2		BUS_WIDTH_16

	#define SPEED_BIN_DDR_DB_68XX		SPEED_BIN_DDR_2400R
	#define BUS_WIDTH_DB_68XX			BUS_WIDTH_16
	#define SPEED_BIN_DDR_DB_BP_6821	SPEED_BIN_DDR_2400R
	#define BUS_WIDTH_DB_BP_6821		BUS_WIDTH_16
#endif


#ifdef CONFIG_CUSTOMER_BOARD_SUPPORT
/************************************* Customer Boards Topology *************************************/
MV_HWS_TOPOLOGY_MAP TopologyMap[] =
{
    /* 1st Customer board - reference from DB */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_CUST_BOARD_1, BUS_WIDTH_CUST_BOARD_1 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* 2nd Customer board  - GP Board*/
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_CUST_BOARD_2, BUS_WIDTH_CUST_BOARD_2 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* 3rd Customer board  -  SolidRun ClearFog-A1 REV2.0 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_CLEARFOG_BOARD, BUS_WIDTH_CLEARFOG_BOARD , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* 4th Customer board  -  Technologic Systems TS-7800v2 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1600K, BUS_WIDTH_16 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* 5th Customer board  -  Technologic Systems TS-7840 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1600K, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_533, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT_ECC  /* Buses mask */
    }
};

#else /* CONFIG_CUSTOMER_BOARD_SUPPORT */

/************************************* Marvell Boards Topology *************************************/
MV_HWS_TOPOLOGY_MAP TopologyMap[] =
{
    /* Marvell board - Board_ID = RD_NAS_68XX_ID = 0*/
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                                                     speed_bin        memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1866L, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* Marvell board - Board_ID = DB_68XX_ID = 1 (DDR3/4)*/
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x3,2,0,0}, {0x3,2,0,0}, {0x3,2,0,0}, {0x3,2,0,0}, {0x3,2,0,0}}, SPEED_BIN_DDR_DB_68XX, BUS_WIDTH_DB_68XX , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* Marvell board - Board_ID = RD_AP_68XX_ID = 2 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1866L, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* Marvell board - Board_ID = DB_AP_68XX_ID = 3 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1866L, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* Marvell board - Board_ID = DB_GP_68XX_ID = 4 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                                                     speed_bin        memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1866L, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
    /* Marvell board - Board_ID = DB_BP_6821_ID = 5 (DDR3/4) */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                                                     speed_bin        memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0x1,0,0}, {0x1,0x1,0,0}, {0x1,0x1,0,0}, {0x1,0x1,0,0}, {0x1,0x1,0,0}}, SPEED_BIN_DDR_DB_BP_6821, BUS_WIDTH_DB_BP_6821, MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
    INTERFACE_BUS_MASK_16BIT  /* Buses mask */
    },
	/* Marvell board - Board_ID = DB_AMC_6820_ID = 6 */
    {
    0x1, /* active interfaces */
    /*cs_mask, mirror, dqs_swap, ck_swap X PUPs                                     speed_bin             memory_device_width  mem_size     frequency  casL casWL      temperature */
	{{{{0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}, {0x1,0,0,0}}, SPEED_BIN_DDR_1866L, BUS_WIDTH_8 , MEM_4G, DDR_FREQ_800, 0 ,   0 , MV_HWS_TEMP_LOW}},
	INTERFACE_BUS_MASK_32BIT  /* Buses mask */
    },
};
#endif /* CONFIG_CUSTOMER_BOARD_SUPPORT */

#endif /* _INC_A38X_TOPOLOGY_H */
