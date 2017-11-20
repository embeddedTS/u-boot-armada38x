/*******************************************************************************
Copyright (C) 2016 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
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

#include "ddr3_init.h"
#include "mv_ddr_xor_v2.h"
#include "mv_ddr_validate.h"

#define DDR_INTERFACES_NUM		1
#define DDR_INTERFACE_OCTETS_NUM	9

#if defined(CONFIG_DDR4)
u16 odt_slope[] = {
	21443,
	1452,
	482,
	240,
	141,
	90,
	67,
	52
};

u16 odt_intercept[] = {
	1517,
	328,
	186,
	131,
	100,
	80,
	69,
	61
};

/* Map of scratch PHY registers used to store stability value */
/* FIXME: the following array is a placeholder; fill it with appropriate values */
u32 dmin_phy_reg_table[MAX_BUS_NUM * MAX_CS_NUM][2] = {
	/* subphy, addr */
	{0, 0xc0},	/* cs 0, subphy 0 */
	{0, 0xc1},	/* cs 0, subphy 1 */
	{0, 0xc2},	/* cs 0, subphy 2 */
	{0, 0xc3},	/* cs 0, subphy 3 */
	{0, 0xc4},	/* cs 0, subphy 4 */
	{1, 0xc0},	/* cs 0, subphy 5 */
	{1, 0xc1},	/* cs 0, subphy 6 */
	{1, 0xc2},	/* cs 0, subphy 7 */
	{1, 0xc3},	/* cs 0, subphy 8 */
	{1, 0xc4},	/* cs 1, subphy 0 */
	{2, 0xc0},	/* cs 1, subphy 1 */
	{2, 0xc1},	/* cs 1, subphy 2 */
	{2, 0xc2},	/* cs 1, subphy 3 */
	{2, 0xc3},	/* cs 1, subphy 4 */
	{2, 0xc4},	/* cs 1, subphy 5 */
	{0, 0xc5},	/* cs 1, subphy 6 */
	{1, 0xc5},	/* cs 1, subphy 7 */
	{2, 0xc5},	/* cs 1, subphy 8 */
	{0, 0xc6},	/* cs 2, subphy 0 */
	{1, 0xc6},	/* cs 2, subphy 1 */
	{0, 0xc0},	/* cs 2, subphy 2 */
	{0, 0xc1},	/* cs 2, subphy 3 */
	{0, 0xc2},	/* cs 2, subphy 4 */
	{0, 0xc3},	/* cs 2, subphy 5 */
	{0, 0xc4},	/* cs 2, subphy 6 */
	{0, 0xc0},	/* cs 2, subphy 7 */
	{0, 0xc1},	/* cs 2, subphy 8 */
	{0, 0xc2},	/* cs 3, subphy 0 */
	{0, 0xc3},	/* cs 3, subphy 1 */
	{0, 0xc4},	/* cs 3, subphy 2 */
	{0, 0xc0},	/* cs 3, subphy 3 */
	{0, 0xc1},	/* cs 3, subphy 4 */
	{0, 0xc2},	/* cs 3, subphy 5 */
	{0, 0xc3},	/* cs 3, subphy 6 */
	{0, 0xc4},	/* cs 3, subphy 7 */
	{0, 0xc4}	/* cs 3, subphy 8 */
};
#endif /* CONFIG_DDR4 */

static u32 dq_bit_map_2_phy_pin[LAST_DDR_CFG_OPT][MAX_DQ_NUM] = {
	{/* LPDDR4_DIMM */},
	{/* LPDDR4_BRD */},
	{/* DDR4_DIMM */
	 /*DQ0	DQ1	DQ2	DQ3	DQ4	DQ5	DQ6	DQ7	DM*/
	 0,	7,	2,	6,	8,	10,	9,	1,/* 3,phy 0 */
	 6,	2,	1,	0,	8,	9,	10,	7,/* 3,phy 1 */
	 1,	2,	0,	6,	8,	9,	10,	7,/* 3,phy 2 */
	 1,	6,	0,	2,	8,	9,	10,	7,/* 3,phy 3 */
	 0,	2,	1,	6,	7,	8,	9,	10,/* 3,phy 4 */
	 0,	2,	1,	6,	7,	8,	9,	10,/* 3,phy 5 */
	 0,	6,	1,	2,	7,	8,	9,	10,/* 3,phy 6 */
	 0,	1,	2,	6,	7,	8,	9,	10,/* 3,phy 7 */
	 9,	0,	8,	7,	6,	10,	2,	1/* 3 phy 8 - ECC */},
	{/* DDR4_BRD */
	 0,	1,	2,	7,	10,	9,	8,	6,/* 3,phy 0 */
	 8,	10,	1,	2,	0,	7,	9,	6,/* 3,phy 1 */
	 9,	10,	2,	7,	0,	1,	6,	8,/* 3,phy 2 */
	 2,	1,	6,	0,	8,	10,	7,	9,/* 3,phy 3 */
	 0,	0,	0,	0,	0,	0,	0,	0,/* 0,phy 4 */
	 0,	0,	0,	0,	0,	0,	0,	0,/* 0,phy 5 */
	 0,	0,	0,	0,	0,	0,	0,	0,/* 0,phy 6 */
	 0,	0,	0,	0,	0,	0,	0,	0,/* 0,phy 7 */
	 2,	8,	0,	9,	6,	7,	10,	1/* 3 phy 8 - ECC */},
	{/* DDR3_DIMM */},
	{/* DDR3_BRD */}
};

static u8 bw_per_freq[DDR_FREQ_LAST] = {
	0x3,	/* DDR_FREQ_100 */
	0x5,	/* DDR_FREQ_650 */
	0x5,	/* DDR_FREQ_667 */
	0x5,	/* DDR_FREQ_800 */
	0x5,	/* DDR_FREQ_933 */
	0x5,	/* DDR_FREQ_1066 */
	0x5,	/* DDR_FREQ_900 */
	0x5,	/* DDR_FREQ_1000 */
	0x5,	/* DDR_FREQ_1050 */
	0x5,	/* DDR_FREQ_1200 */
};

static u8 rate_per_freq[DDR_FREQ_LAST] = {
	0x1,	/* DDR_FREQ_100 */
	0x2,	/* DDR_FREQ_650 */
	0x2,	/* DDR_FREQ_667 */
	0x2,	/* DDR_FREQ_800 */
	0x3,	/* DDR_FREQ_933 */
	0x3,	/* DDR_FREQ_1066 */
	0x3,	/* DDR_FREQ_900 */
	0x3,	/* DDR_FREQ_1000 */
	0x3,	/* DDR_FREQ_1050 */
	0x3,	/* DDR_FREQ_1200 */
};

/*
 * Accessor functions for the registers
 */
void reg_write(u32 addr, u32 val)
{
	mmio_write_32(INTER_REGS_BASE + addr, val);
}

u32 reg_read(u32 addr)
{
	return mmio_read_32(INTER_REGS_BASE + addr);
}

void reg_bit_set(u32 addr, u32 mask)
{
	mmio_write_32(INTER_REGS_BASE + addr,
		      mmio_read_32(INTER_REGS_BASE + addr) | mask);
}

void reg_bit_clr(u32 addr, u32 mask)
{
	mmio_write_32(INTER_REGS_BASE + addr,
		      mmio_read_32(INTER_REGS_BASE + addr) & ~mask);
}

void reg_bit_clrset(u32 addr, u32 val, u32 mask)
{
	mmio_clrsetbits_32(INTER_REGS_BASE + addr, mask, val);
}

void mmio_write2_32(u32 val, u32 addr)
{
	mmio_write_32(addr, val);
}

/* remap overlapping dram region to the top */
#define ALIAS_BANKED_UID(n)	(0x800 + 0x400 * (n))
#define DRAM_UID		3
#define CCU_MC_RCR_BASE		0x300
#define CCU_MC_RCR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RCR_BASE)
#define REMAP_EN_ENA		1
#define REMAP_EN_OFFS		0
#define REMAP_EN_MASK		0x1
#define REMAP_SIZE_OFFS		20
#define REMAP_SIZE_MASK		0xfff
#define CCU_MC_RSBR_BASE	0x304
#define CCU_MC_RSBR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RSBR_BASE)
#define SOURCE_BASE_OFFS	10
#define SOURCE_BASE_MASK	0xfffff
#define CCU_MC_RTBR_BASE	0x308
#define CCU_MC_RTBR_REG		(ALIAS_BANKED_UID(DRAM_UID) + CCU_MC_RTBR_BASE)
#define TARGET_BASE_OFFS	10
#define TARGET_BASE_MASK	0xfffff
int mv_ddr_mc_remap(void)
{
	uintptr_t remap_target = NON_DRAM_MEM_RGN_START_ADDR;
	uintptr_t remap_source = mv_ddr_get_total_memory_size_in_bits() / MV_DDR_NUM_BITS_IN_BYTE;
	uint64_t remap_size = NON_DRAM_MEM_RGN_END_ADDR - NON_DRAM_MEM_RGN_START_ADDR;
	u32 val;

	if (remap_source < NON_DRAM_MEM_RGN_START_ADDR)
		return MV_OK;

	if ((remap_size == 0) ||		/* can't be zero */
	    (remap_size >> 32) ||		/* can't be more than 4GB */
	    (remap_size % (1 << 20))) {		/* must be multiple of 1MB */
		printf("%s: incorrect mc remap size found\n", __func__);
		return MV_FAIL;
	}

	if (remap_target % remap_size) {	/* must be multiple of remap size */
		printf("%s: incorrect mc remap target base addr found\n", __func__);
		return MV_FAIL;
	}

	if (remap_source % remap_size) {	/* must be multiple of remap size */
		printf("%s: incorrect mc remap source base addr found\n", __func__);
		return MV_FAIL;
	}

	/* set mc remap source base to the top of dram */
	remap_source >>= 20;	/* in MB */
	val = (remap_source & SOURCE_BASE_MASK) << SOURCE_BASE_OFFS;
	reg_write(CCU_MC_RSBR_REG, val);

	/* set mc remap target base to the overlapping dram region */
	remap_target >>= 20;	/* in MB */
	val = (remap_target & TARGET_BASE_MASK) << TARGET_BASE_OFFS;
	reg_write(CCU_MC_RTBR_REG, val);

	/* set mc remap size to the size of the overlapping dram region */
	remap_size >>= 20;	/* in MB */
	val = ((remap_size - 1) & REMAP_SIZE_MASK) << REMAP_SIZE_OFFS;
	/* enable remapping */
	val |= (REMAP_EN_ENA << REMAP_EN_OFFS);
	reg_write(CCU_MC_RCR_REG, val);

	return MV_OK;
}

void mv_ddr_mem_scrubbing(void)
{
	uint64_t val = 0;
	uint64_t tot_mem_sz;

	tot_mem_sz = mv_ddr_get_total_memory_size_in_bits() / MV_DDR_NUM_BITS_IN_BYTE;

	printf("mv_ddr: scrubbing memory...\n");

	/* scrub memory up to non-dram memory region */
	if (tot_mem_sz < NON_DRAM_MEM_RGN_START_ADDR)
		mv_ddr_dma_memset(0, tot_mem_sz, val);
	else
		mv_ddr_dma_memset(0, NON_DRAM_MEM_RGN_START_ADDR, val);

	/* scrub memory up to the end */
	if (tot_mem_sz >= NON_DRAM_MEM_RGN_END_ADDR)
		mv_ddr_dma_memset(NON_DRAM_MEM_RGN_END_ADDR,
				  tot_mem_sz - NON_DRAM_MEM_RGN_START_ADDR, val);
}

static u8 mv_ddr_tip_clk_ratio_get(u32 freq)
{
	if ((freq == DDR_FREQ_LOW_FREQ) || (freq_val[freq] <= 400))
		return 1;

	return 2;
}


/*
 * Name:	mv_ddr_tip_freq_config_get
 * Desc:
 * Args:
 * Notes:
 * Returns:	MV_OK if success, other error code if fail
 */
static int mv_ddr_tip_freq_config_get(u8 dev_num, enum hws_ddr_freq freq,
				      struct hws_tip_freq_config_info
					*freq_config_info)
{
	if (freq_config_info == NULL)
		return MV_BAD_PARAM;

	freq_config_info->bw_per_freq = bw_per_freq[freq];
	freq_config_info->rate_per_freq = rate_per_freq[freq];
	freq_config_info->is_supported = 1;

	return MV_OK;
}

static void dunit_read(u32 addr, u32 mask, u32 *data)
{
	*data = reg_read(addr + DUNIT_BASE_ADDR) & mask;
}

static void dunit_write(u32 addr, u32 mask, u32 data)
{
	u32 reg_val = data;

	if (mask != MASK_ALL_BITS) {
		dunit_read(addr, MASK_ALL_BITS, &reg_val);
		reg_val &= (~mask);
		reg_val |= (data & mask);
	}

	reg_write(addr + DUNIT_BASE_ADDR, reg_val);
}

void mv_ddr_odpg_enable(void)
{
	dunit_write(ODPG_DATA_CTRL_REG,
		    ODPG_ENABLE_MASK << ODPG_ENABLE_OFFS,
		    ODPG_ENABLE_ENA << ODPG_ENABLE_OFFS);
}

void mv_ddr_odpg_disable(void)
{
	dunit_write(ODPG_DATA_CTRL_REG,
		    ODPG_DISABLE_MASK << ODPG_DISABLE_OFFS,
		    ODPG_DISABLE_DIS << ODPG_DISABLE_OFFS);
}

void mv_ddr_odpg_done_clr(void)
{
	dunit_write(ODPG_DONE_STATUS_REG,
		    ODPG_DONE_STATUS_BIT_MASK << ODPG_DONE_STATUS_BIT_OFFS,
		    ODPG_DONE_STATUS_BIT_CLR << ODPG_DONE_STATUS_BIT_OFFS);
}

int mv_ddr_is_odpg_done(u32 count)
{
	u32 i, data;

	for (i = 0; i < count; i++) {
		dunit_read(ODPG_DONE_STATUS_REG, MASK_ALL_BITS, &data);
		if (((data >> ODPG_DONE_STATUS_BIT_OFFS) & ODPG_DONE_STATUS_BIT_MASK) ==
		     ODPG_DONE_STATUS_BIT_SET)
			break;
	}

	if (i >= count) {
		printf("%s: timeout\n", __func__);
		return MV_FAIL;
	}

	mv_ddr_odpg_done_clr();

	return MV_OK;
}

void mv_ddr_training_enable(void)
{
	dunit_write(GLOB_CTRL_STATUS_REG,
		    TRAINING_TRIGGER_MASK << TRAINING_TRIGGER_OFFS,
		    TRAINING_TRIGGER_ENA << TRAINING_TRIGGER_OFFS);
}

int mv_ddr_is_training_done(u32 count, u32 *result)
{
	u32 i, data;

	if (result == NULL) {
		printf("%s: NULL result pointer found\n", __func__);
		return MV_FAIL;
	}

	for (i = 0; i < count; i++) {
		dunit_read(GLOB_CTRL_STATUS_REG, MASK_ALL_BITS, &data);
		if (((data >> TRAINING_DONE_OFFS) & TRAINING_DONE_MASK) ==
		     TRAINING_DONE_DONE)
			break;
	}

	if (i >= count) {
		printf("%s: timeout\n", __func__);
		return MV_FAIL;
	}

	*result = (data >> TRAINING_RESULT_OFFS) & TRAINING_RESULT_MASK;

	return MV_OK;
}

#define DM_PAD	3
u32 mv_ddr_dm_pad_get()
{
	return DM_PAD;
}


/* return ddr frequency from sar */
static int mv_ddr_sar_freq_get(int dev_num, enum hws_ddr_freq *freq)
{
	u32 ddr_clk_config;

	/* Read ddr clk config from sar */
	ddr_clk_config = (reg_read(SAR_REG_ADDR) >>
		RST2_CLOCK_FREQ_MODE_OFFS) &
		RST2_CLOCK_FREQ_MODE_MASK;

	switch (ddr_clk_config) {
	case CPU_2000_DDR_1200_RCLK_1200:
		*freq = DDR_FREQ_1200;
		break;
	case CPU_2000_DDR_1050_RCLK_1050:
		*freq = DDR_FREQ_1050;
		break;
	case CPU_1600_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_1800_DDR_1200_RCLK_1200:
		*freq = DDR_FREQ_1200;
		break;
	case CPU_1800_DDR_1050_RCLK_1050:
		*freq = DDR_FREQ_1050;
		break;
	case CPU_1600_DDR_1050_RCLK_1050:
		*freq = DDR_FREQ_1050;
		break;
	case CPU_1000_DDR_650_RCLK_650:
		*freq = DDR_FREQ_650;
		break;
	case CPU_1300_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_1300_DDR_650_RCLK_650:
		*freq = DDR_FREQ_650;
		break;
	case CPU_1200_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_1400_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_600_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_800_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	case CPU_1000_DDR_800_RCLK_800:
		*freq = DDR_FREQ_800;
		break;
	default:
		*freq = 0;
		return MV_NOT_SUPPORTED;
	}

	return MV_OK;
}

/*
 * Name:	mv_ddr_target_div_calc
 * Desc:	calculates and returns target mc clk divider value
 * Args:
		curr_div -  current mc clk divider value
		curr_freq  - current frequency configured to
		target_freq - target frequency to step to
 * Notes:
 * Returns:	target mc clk divider value
 */
static u32 mv_ddr_target_div_calc(u32 curr_div, u32 curr_freq, u32 target_freq)
{
	u32 target_div;

	target_div = (curr_freq * curr_div) / target_freq;

	return target_div;
}

/*
 * Name:	mv_ddr_clk_dividers_set
 * Desc:	sets target frequency
		- changes clk dividers for mc6 and dunit
		  and executes DFS procedure.
		- dunit and mc6 freqs to be configured;
		  other freqs (e.g., hclk, fclk) are derivatives.
		- the DFS flow is as follows:
		  -> save specific SoC configurations (e.g., ODT),
		  -> turn DLL off (no need at low freq)
		  -> put memory in self-refresh mode
 * Args:
		ddr target frequency
 * Notes:
 * Returns:	status MV_OK or fail
 */
static int mv_ddr_clk_dividers_set(u8 dev_num, u32 if_id, enum hws_ddr_freq target_ddr_freq)
{
	static u32 ddr_div = 0xffffffff;
	u32 mc_target_div, ddr_target_div;
	static u32 init_ddr_freq_val;
	u32 target_ddr_freq_val;
	static int mv_ddr_first_time_setting = 1;
	u32 reg;
	enum hws_ddr_freq init_ddr_freq;

	if (if_id != 0) {
		DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
			              ("mv_ddr: a70x0: interface 0x%x not supported\n",
				       if_id));
		return MV_BAD_PARAM;
	}

	if (mv_ddr_first_time_setting) {
		/* get ddr init freq */
		init_ddr_freq = mv_ddr_init_freq_get();

		init_ddr_freq_val = freq_val[init_ddr_freq];

		/* get mc & ddr clk dividers values */
		reg = reg_read(DEV_GEN_CTRL1_REG_ADDR);
		ddr_div = (reg >> MISC_CLKDIV_RATIO_1_OFFS) & MISC_CLKDIV_RATIO_1_MASK;

		mv_ddr_first_time_setting = 0;
	}

	target_ddr_freq_val = freq_val[target_ddr_freq];

	/* calc mc & ddr target clk divider value */
	ddr_target_div = mv_ddr_target_div_calc(ddr_div, init_ddr_freq_val, target_ddr_freq_val);
	mc_target_div = ddr_target_div * 2;

	reg = reg_read(DEV_GEN_CTRL1_REG_ADDR);
	reg &= ~(MISC_CLKDIV_RATIO_2_MASK << MISC_CLKDIV_RATIO_2_OFFS | MISC_CLKDIV_RATIO_1_MASK << MISC_CLKDIV_RATIO_1_OFFS);
	reg |= mc_target_div << MISC_CLKDIV_RATIO_2_OFFS | ddr_target_div << MISC_CLKDIV_RATIO_1_OFFS;
	reg_write(DEV_GEN_CTRL1_REG_ADDR, reg);

	/* Reload force, relax enable, align enable set */
	reg = reg_read(DEV_GEN_CTRL3_REG_ADDR);
	reg &= ~(MISC_CLKDIV_RELOAD_FORCE_MASK << MISC_CLKDIV_RELOAD_FORCE_OFFS |
		 MISC_CLKDIV_RELAX_EN_MASK << MISC_CLKDIV_RELAX_EN_OFFS |
		 MISC_CLKDIV_ALIGN_EN_MASK << MISC_CLKDIV_ALIGN_EN_OFFS);
	reg |= RELOAD_FORCE_VAL << MISC_CLKDIV_RELOAD_FORCE_OFFS |
	       RELAX_EN_VAL << MISC_CLKDIV_RELAX_EN_OFFS |
	       ALIGN_EN_VAL << MISC_CLKDIV_ALIGN_EN_OFFS;
	reg_write(DEV_GEN_CTRL3_REG_ADDR, reg);

	/* Reload smooth */
	reg = reg_read(DEV_GEN_CTRL4_REG_ADDR);
	reg &= ~(MISC_CLKDIV_RELOAD_SMOOTH_MASK << MISC_CLKDIV_RELOAD_SMOOTH_OFFS);
	reg |= RELOAD_SMOOTH_VAL << MISC_CLKDIV_RELOAD_SMOOTH_OFFS;
	reg_write(DEV_GEN_CTRL4_REG_ADDR, reg);

	/* Toggle reload ratio first 0x1 then 0x0*/
	reg = reg_read(DEV_GEN_CTRL4_REG_ADDR);
	reg &= ~(MISC_CLKDIV_RELOAD_RATIO_MASK << MISC_CLKDIV_RELOAD_RATIO_OFFS);
	reg |= RELOAD_RATIO_VAL << MISC_CLKDIV_RELOAD_RATIO_OFFS;
	reg_write(DEV_GEN_CTRL4_REG_ADDR, reg);

	/* delay between toggles */
	mdelay(10); /* TODO: check the delay value */
	reg = reg_read(DEV_GEN_CTRL4_REG_ADDR);
	reg &= ~(MISC_CLKDIV_RELOAD_RATIO_MASK << MISC_CLKDIV_RELOAD_RATIO_OFFS);
	reg_write(DEV_GEN_CTRL4_REG_ADDR, reg);

	/* Unblock phase_sync_mc_clk in RFU */
	reg = reg_read(CLKS_CTRL_REG_ADDR);
	reg &= ~(BLOCK_PHI_RST_TO_RING_TO_MC_CLK_MASK << BLOCK_PHI_RST_TO_RING_TO_MC_CLK_OFFS);
	reg_write(CLKS_CTRL_REG_ADDR, reg);

	/* delay between toggles */
	mdelay(10); /* TODO: check the delay value */

	/* Ring-MC clock f2s reset toggle */
	reg = reg_read(CLKS_CTRL_REG_ADDR);
	reg &= ~(RING_CLK_TO_ALL_CLK_PHI_RST_MASK << RING_CLK_TO_ALL_CLK_PHI_RST_OFFS);
	reg_write(CLKS_CTRL_REG_ADDR, reg);

	/* delay between toggles */
	mdelay(100); /* TODO: check the delay value */

	reg = reg_read(CLKS_CTRL_REG_ADDR);
	reg |= (RING_CLK_TO_ALL_CLK_PHI_RST_MASK << RING_CLK_TO_ALL_CLK_PHI_RST_OFFS);
	reg_write(CLKS_CTRL_REG_ADDR, reg);

	mdelay(50); /* TODO: check the delay value */

	reg = reg_read(CLKS_CTRL_REG_ADDR);
	reg |= (BLOCK_PHI_RST_TO_RING_TO_MC_CLK_MASK << BLOCK_PHI_RST_TO_RING_TO_MC_CLK_OFFS);
	reg_write(CLKS_CTRL_REG_ADDR, reg);

	return MV_OK;
}

/*
 * Name:     ddr3_tip_apn806_select_ddr_controller.
 * Desc:     Enable/Disable access to Marvell's server.
 * Args:     dev_num     - device number
 *           enable        - whether to enable or disable the server
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_tip_apn806_select_ddr_controller(u8 dev_num, int enable)
{
	u32 reg;

	reg = reg_read(DUAL_DUNIT_CFG_REG);

	reg &= ~(TUNING_ACTIVE_SEL_MASK << TUNING_ACTIVE_SEL_OFFS);
	reg |= (enable << TUNING_ACTIVE_SEL_OFFS);

	reg_write(DUAL_DUNIT_CFG_REG, reg);

	return MV_OK;
}

/*
 * external read from memory
 */
int ddr3_tip_ext_read(u32 dev_num, u32 if_id, u32 reg_addr,
		      u32 num_of_bursts, u32 *data)
{
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		data[burst_num] = readl(reg_addr + 4 * burst_num);

	return MV_OK;
}

/*
 * external write to memory
 */
int ddr3_tip_ext_write(u32 dev_num, u32 if_id, u32 reg_addr,
		       u32 num_of_bursts, u32 *data) {
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		writel(data[burst_num], reg_addr + 4 * burst_num);

	return MV_OK;
}

#ifdef MV_DDR_PRFA
/* check indirect access to phy register file completed */
static int is_prfa_done(void)
{
	u32 reg_val;
	u32 iter = 0;

	do {
		if (iter++ > MAX_POLLING_ITERATIONS) {
			printf("error: %s: polling timeout\n", __func__);
			return MV_FAIL;
		}
		dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
		reg_val >>= PRFA_REQ_OFFS;
		reg_val &= PRFA_REQ_MASK;
	} while (reg_val == PRFA_REQ_ENA); /* request pending */

	return MV_OK;
}

/* write to phy register thru indirect access */
static int prfa_write(enum hws_access_type phy_access, u32 phy,
		      enum hws_ddr_phy phy_type, u32 addr,
		      u32 data, enum hws_operation op_type)
{
	u32 reg_val = ((data & PRFA_DATA_MASK) << PRFA_DATA_OFFS) |
		      ((addr & PRFA_REG_NUM_MASK) << PRFA_REG_NUM_OFFS) |
		      ((phy & PRFA_PUP_NUM_MASK) << PRFA_PUP_NUM_OFFS) |
		      ((phy_type & PRFA_PUP_CTRL_DATA_MASK) << PRFA_PUP_CTRL_DATA_OFFS) |
		      ((phy_access & PRFA_PUP_BCAST_WR_ENA_MASK) << PRFA_PUP_BCAST_WR_ENA_OFFS) |
		      (((addr >> 6) & PRFA_REG_NUM_HI_MASK) << PRFA_REG_NUM_HI_OFFS) |
		      ((op_type & PRFA_TYPE_MASK) << PRFA_TYPE_OFFS);
	dunit_write(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, reg_val);
	reg_val |= (PRFA_REQ_ENA << PRFA_REQ_OFFS);
	dunit_write(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, reg_val);

	/* polling for prfa request completion */
	if (is_prfa_done() != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/* read from phy register thru indirect access */
static int prfa_read(enum hws_access_type phy_access, u32 phy,
		     enum hws_ddr_phy phy_type, u32 addr, u32 *data)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 max_phy = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 i, reg_val;

	if (phy_access == ACCESS_TYPE_MULTICAST) {
		for (i = 0; i < max_phy; i++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
			if (prfa_write(ACCESS_TYPE_UNICAST, i, phy_type, addr, 0, OPERATION_READ) != MV_OK)
				return MV_FAIL;
			dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
			data[i] = (reg_val >> PRFA_DATA_OFFS) & PRFA_DATA_MASK;
		}
	} else {
		if (prfa_write(phy_access, phy, phy_type, addr, 0, OPERATION_READ) != MV_OK)
			return MV_FAIL;
		dunit_read(PHY_REG_FILE_ACCESS_REG, MASK_ALL_BITS, &reg_val);
		*data = (reg_val >> PRFA_DATA_OFFS) & PRFA_DATA_MASK;
	}

	return MV_OK;
}
#else /* MV_DDR_PRDA */
#define PRDA_DATA_OFFS			0
#define PRDA_DATA_MASK			0xffff
#define PRDA_BIT15			0x1
#define PRDA_BIT15_OFFS			15
#define PRDA_BIT15_MASK			0x1
#define PRDA_REG_NUM_OFFS		2
#define PRDA_REG_NUM_MASK		0xff
#define PRDA_PUP_NUM_OFFS		10
#define PRDA_PUP_NUM_MASK		0xf
#define PRDA_PUP_CTRL_DATA_OFFS		14
#define PRDA_PUP_CTRL_DATA_MASK		0x1
#define PRDA_PUP_CTRL_BCAST		0x1e
#define PRDA_PUP_DATA_BCAST		0x1d
/* write to phy register thru direct access */
static int prda_write(enum hws_access_type phy_access, u32 phy,
		     enum hws_ddr_phy phy_type, u32 addr,
		     u32 data, enum hws_operation op_type)
{
	u32 reg_addr = (PRDA_BIT15 << PRDA_BIT15_OFFS) |
		       ((addr & PRDA_REG_NUM_MASK) << PRDA_REG_NUM_OFFS);
	u32 reg_val = (data & PRDA_DATA_MASK) << PRDA_DATA_OFFS;

	if (phy_access == ACCESS_TYPE_MULTICAST) {
		if (phy_type == DDR_PHY_DATA)
			reg_addr |= (PRDA_PUP_DATA_BCAST << PRDA_PUP_NUM_OFFS);
		else
			reg_addr |= (PRDA_PUP_CTRL_BCAST << PRDA_PUP_NUM_OFFS);
	} else { /* unicast access type */
		reg_addr |= ((phy & PRDA_PUP_NUM_MASK) << PRDA_PUP_NUM_OFFS);
		reg_addr |= ((phy_type & PRDA_PUP_CTRL_DATA_MASK) <<
			     PRDA_PUP_CTRL_DATA_OFFS);
	}

	dunit_write(reg_addr, MASK_ALL_BITS, reg_val);

	return MV_OK;
}

/* read from phy register thry direct access */
static int prda_read(enum hws_access_type phy_access, u32 phy,
		    enum hws_ddr_phy phy_type, u32 addr, u32 *data)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 max_phy = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	u32 i, reg_val;
	u32 reg_addr = (PRDA_BIT15 << PRDA_BIT15_OFFS) |
		       ((addr & PRDA_REG_NUM_MASK) << PRDA_REG_NUM_OFFS);

	if (phy_access == ACCESS_TYPE_MULTICAST) {
		for (i = 0; i < max_phy; i++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, i);
			reg_addr |= ((i & PRDA_PUP_NUM_MASK) << PRDA_PUP_NUM_OFFS);
			reg_addr |= ((phy_type & PRDA_PUP_CTRL_DATA_MASK) <<
				     PRDA_PUP_CTRL_DATA_OFFS);
			dunit_read(reg_addr, MASK_ALL_BITS, &reg_val);
			data[i] = (reg_val >> PRDA_DATA_OFFS) & PRDA_DATA_MASK;
			reg_addr = (PRDA_BIT15 << PRDA_BIT15_OFFS) |
				   ((addr & PRDA_REG_NUM_MASK) << PRDA_REG_NUM_OFFS);
		}
	} else { /* unicast type access */
		reg_addr |= ((phy & PRDA_PUP_NUM_MASK) << PRDA_PUP_NUM_OFFS);
		reg_addr |= ((phy_type & PRDA_PUP_CTRL_DATA_MASK) <<
			     PRDA_PUP_CTRL_DATA_OFFS);
		dunit_read(reg_addr, MASK_ALL_BITS, &reg_val);
		*data = (reg_val >> PRDA_DATA_OFFS) & PRDA_DATA_MASK;
	}

	return MV_OK;
}
#endif

static int mv_ddr_sw_db_init(u32 dev_num, u32 board_id)
{
	struct hws_tip_config_func_db config_func;
#if !defined(CONFIG_DDR4)
	enum hws_ddr_freq ddr_freq = DDR_FREQ_LOW_FREQ;
#endif

	/* new read leveling version */
	config_func.mv_ddr_dunit_read = dunit_read;
	config_func.mv_ddr_dunit_write = dunit_write;
	config_func.tip_dunit_mux_select_func =
		ddr3_tip_apn806_select_ddr_controller;
	config_func.tip_get_freq_config_info_func = mv_ddr_tip_freq_config_get;
	config_func.tip_set_freq_divider_func = mv_ddr_clk_dividers_set;
	config_func.tip_get_device_info_func = NULL;
	config_func.tip_get_temperature = NULL;
	config_func.tip_get_clock_ratio = mv_ddr_tip_clk_ratio_get;
	config_func.tip_external_read = ddr3_tip_ext_read;
	config_func.tip_external_write = ddr3_tip_ext_write;
#ifdef MV_DDR_PRFA
	config_func.mv_ddr_phy_read = prfa_read;
	config_func.mv_ddr_phy_write = prfa_write;
#else
	config_func.mv_ddr_phy_read = prda_read;
	config_func.mv_ddr_phy_write = prda_write;
#endif
	ddr3_tip_init_config_func(dev_num, &config_func);

#if defined(A80X0)
	ddr3_tip_register_dq_table(dev_num, dq_bit_map_2_phy_pin[DDR4_DIMM]);
#else
	ddr3_tip_register_dq_table(dev_num, dq_bit_map_2_phy_pin[DDR4_BRD]);
#endif
	/* set device attributes*/
	ddr3_tip_dev_attr_init(dev_num);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_TIP_REV, MV_TIP_REV_4);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_PHY_EDGE, MV_DDR_PHY_EDGE_NEGATIVE);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_OCTET_PER_INTERFACE, DDR_INTERFACE_OCTETS_NUM);
	ddr3_tip_dev_attr_set(dev_num, MV_ATTR_INTERLEAVE_WA, 0);

	ca_delay = 0;
	delay_enable = 1;
	dfs_low_freq = DFS_LOW_FREQ_VALUE;
	calibration_update_control = 1;

	return MV_OK;
}

static int mv_ddr_training_mask_set(void)
{
#if defined(CONFIG_DDR4)
	mask_tune_func = (SET_LOW_FREQ_MASK_BIT |
			  WRITE_LEVELING_LF_MASK_BIT |
			  LOAD_PATTERN_MASK_BIT |
			  SET_TARGET_FREQ_MASK_BIT |
			  WRITE_LEVELING_TF_MASK_BIT |
			  WL_PHASE_CORRECTION_MASK_BIT |
			  RL_DQS_BURST_MASK_BIT |
			  RECEIVER_CALIBRATION_MASK_BIT |
			  DQ_VREF_CALIBRATION_MASK_BIT);
	rl_mid_freq_wa = 0;
#else /* CONFIG_DDR4 */
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	enum hws_ddr_freq ddr_freq = tm->interface_params[0].memory_freq;

	mask_tune_func = (SET_LOW_FREQ_MASK_BIT |
			  LOAD_PATTERN_MASK_BIT |
			  SET_MEDIUM_FREQ_MASK_BIT | WRITE_LEVELING_MASK_BIT |
			  WRITE_LEVELING_SUPP_MASK_BIT |
			  READ_LEVELING_MASK_BIT |
			  PBS_RX_MASK_BIT |
			  PBS_TX_MASK_BIT |
			  SET_TARGET_FREQ_MASK_BIT |
			  WRITE_LEVELING_TF_MASK_BIT |
			  WRITE_LEVELING_SUPP_TF_MASK_BIT |
			  READ_LEVELING_TF_MASK_BIT |
			  CENTRALIZATION_RX_MASK_BIT |
			  CENTRALIZATION_TX_MASK_BIT);
	rl_mid_freq_wa = 1;

	if ((ddr_freq == DDR_FREQ_333) || (ddr_freq == DDR_FREQ_400)) {
		mask_tune_func = (WRITE_LEVELING_MASK_BIT |
				  LOAD_PATTERN_2_MASK_BIT |
				  WRITE_LEVELING_SUPP_MASK_BIT |
				  READ_LEVELING_MASK_BIT |
				  PBS_RX_MASK_BIT |
				  PBS_TX_MASK_BIT |
				  CENTRALIZATION_RX_MASK_BIT |
				  CENTRALIZATION_TX_MASK_BIT);
		rl_mid_freq_wa = 0; /* WA not needed if 333/400 is TF */
	}

	/* Supplementary not supported for ECC modes */
	if (1 == ddr3_if_ecc_enabled()) {
		mask_tune_func &= ~WRITE_LEVELING_SUPP_TF_MASK_BIT;
		mask_tune_func &= ~WRITE_LEVELING_SUPP_MASK_BIT;
		mask_tune_func &= ~PBS_TX_MASK_BIT;
		mask_tune_func &= ~PBS_RX_MASK_BIT;
	}
#endif /* CONFIG_DDR4 */

	return MV_OK;
}

int mv_ddr_early_init(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	int rev_id = apn806_rev_id_get();
	int subphy_num;

	/* in case of calibration adjust
	 * this flag checks if to run a workaround where v pod and v sstl are wired
	 */
	vref_calibration_wa = 0;
	mode_2t = 1;
	mv_ddr_sw_db_init(0, 0);

	/* TODO: consider to move this platform specific code into mv_ddr_topology_update() */
	if (rev_id == APN806_REV_ID_A0) {
		/* update the number of cs to be 'single cs in case of A0 */
		for (subphy_num = 0; subphy_num < MAX_BUS_NUM; subphy_num++) {
			tm->interface_params[IF_ID_0].as_bus_params[subphy_num].cs_bitmask = 0x1;
			tm->interface_params[IF_ID_0].as_bus_params[subphy_num].mirror_enable_bitmask = 0x0;
		}
		reg_write(0x114cc, 0x1200d);
		reg_write(0x114c8, 0x1840008);
		reg_write(0x117c8, 0x28a0008);
		reg_write(0x11dc8, 0x1840008);
		reg_write(0x11ec8, 0x28a0008);
	}

	return MV_OK;
}

int mv_ddr_early_init2(void)
{
	mv_ddr_training_mask_set();

	/* remap overlapping dram region to the top */
	if (mv_ddr_mc_remap() != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

#define AVS_DELTA	5
int mv_ddr_pre_training_fixup(void)
{
	u32 reg_val, avs_val;

	/* read and save avs voltage to restore it later */
	nominal_avs = reg_read(AVS_ENABLED_CTRL_REG);
	extension_avs = reg_read(AVS_DISABLED_CTRL2_REG);

	/* update avs voltage for generic run */
	reg_write(AVS_DISABLED_CTRL2_REG, 0xfde1ffff);
	if (apn806_rev_id_get() == APN806_REV_ID_A0) {
		reg_write(AVS_ENABLED_CTRL_REG, 0x1002f2f5);
	} else {
		reg_val = nominal_avs;
		/* extract nominal avs value */
		avs_val = (reg_val >> AVS_LOW_VDD_LMT_OFFS) & AVS_LOW_VDD_LMT_MASK;
		/* reduce nominal avs value */
		avs_val -= AVS_DELTA;
		/* write reduced avs value */
		reg_val &= ~(AVS_LOW_VDD_LMT_MASK << AVS_LOW_VDD_LMT_OFFS);
		reg_val |= ((avs_val & AVS_LOW_VDD_LMT_MASK) << AVS_LOW_VDD_LMT_OFFS);
		reg_val &= ~(AVS_HIGH_VDD_LMT_MASK << AVS_HIGH_VDD_LMT_OFFS);
		reg_val |= ((avs_val & AVS_HIGH_VDD_LMT_MASK) << AVS_HIGH_VDD_LMT_OFFS);
		reg_write(AVS_ENABLED_CTRL_REG, reg_val);
	}

	return 0;
}

int mv_ddr_post_training_fixup(void)
{
	mv_ddr_validate();

	reg_write(AVS_ENABLED_CTRL_REG, nominal_avs);

	return 0;
}

/*
 * Name:     mv_ddr_convert_read_params_from_tip2mc6.
 * Desc:     convert the read ready and the read sample from tip to mc6.
 * Args:
 * Notes:
 * Returns:
 */
static void mv_ddr_convert_read_params_from_tip2mc6(void)
{
	u32	if_id, cs, cl_val, cwl_val, phy_rl_cycle_dly_mc6, rd_smp_dly_tip, phy_rfifo_rptr_dly_val;
	u32	mb_read_data_latency;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 max_cs = ddr3_tip_max_cs_get(DEV_NUM_0);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		cl_val = tm->interface_params[0].cas_l;
		cwl_val = tm->interface_params[0].cas_wl;

		reg_bit_clrset(MC6_CH0_DRAM_CFG1_REG,
			cwl_val << CWL_OFFS | cl_val << CL_OFFS,
			CWL_MASK << CWL_OFFS | CL_MASK << CL_OFFS);

		for (cs = 0; cs < max_cs; cs++) {
			dunit_read(RD_DATA_SMPL_DLYS_REG,
				   MASK_ALL_BITS, &rd_smp_dly_tip);

			rd_smp_dly_tip >>= RD_SMPL_DLY_CS_OFFS(cs);
			rd_smp_dly_tip &= RD_SMPL_DLY_CS_MASK;

			phy_rl_cycle_dly_mc6 = 2 * (rd_smp_dly_tip - cl_val) + 1;

			/* if cl is odd value add 2 else decrease 2 */
			if (cl_val & 0x1)
				phy_rl_cycle_dly_mc6 += 2;
			else
				phy_rl_cycle_dly_mc6 -= 2;

			/* if cwl is odd add 2 */
			if (cwl_val & 0x1)
				phy_rl_cycle_dly_mc6 += 2;

			/* TODO: how to write to mc6 per interface */
			reg_bit_clrset(MC6_CH0_PHY_RL_CTRL_B0_REG(cs),
				       phy_rl_cycle_dly_mc6 << PHY_RL_CYCLE_DLY_OFFS,
				       PHY_RL_CYCLE_DLY_MASK << PHY_RL_CYCLE_DLY_OFFS);
		}
	}

	/* TODO: change these constant initialization below to functions */
	phy_rfifo_rptr_dly_val = 9;	/*FIXME: this parameter should be between 6 to 12 */
	reg_bit_clrset(MC6_CH0_PHY_CTRL1_REG,
		       phy_rfifo_rptr_dly_val << PHY_RFIFO_RPTR_DLY_VAL_OFFS,
		       PHY_RFIFO_RPTR_DLY_VAL_MASK << PHY_RFIFO_RPTR_DLY_VAL_OFFS);

	mb_read_data_latency = 8;	/*FIXME: this parameter should be between 4 to 12 */
	reg_bit_clrset(MC6_RD_DPATH_CTRL_REG,
		       mb_read_data_latency << MB_RD_DATA_LATENCY_CH0_OFFS,
		       MB_RD_DATA_LATENCY_CH0_MASK << MB_RD_DATA_LATENCY_CH0_OFFS);
}

/* precharge ddr banks prior to switching to mc6 */
static int mv_ddr_dunit_pre_charge(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	dunit_write(SDRAM_OP_REG,
		    (SDRAM_OP_CMD_MASK << SDRAM_OP_CMD_OFFS) |
		    (SDRAM_OP_CMD_CS_MASK << SDRAM_OP_CMD_CS_OFFS(0)) |
		    (SDRAM_OP_CMD_CS_MASK << SDRAM_OP_CMD_CS_OFFS(1)) |
		    (SDRAM_OP_CMD_CS_MASK << SDRAM_OP_CMD_CS_OFFS(2)) |
		    (SDRAM_OP_CMD_CS_MASK << SDRAM_OP_CMD_CS_OFFS(3)),
		    ((~tm->interface_params[0].as_bus_params[0].cs_bitmask) <<
		     SDRAM_OP_CMD_CS_OFFS(0)) |
		    (CMD_PRECHARGE << SDRAM_OP_CMD_OFFS));

	if (ddr3_tip_if_polling(0, ACCESS_TYPE_UNICAST, 0, 0, SDRAM_OP_CMD_MASK,
				SDRAM_OP_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
		printf("error: %s: polling timeout\n", __func__);
		return MV_FAIL;
	}

	return MV_OK;
}

int ddr3_post_run_alg(void)
{
	mv_ddr_convert_read_params_from_tip2mc6();
	if (mv_ddr_dunit_pre_charge() != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

int ddr3_silicon_post_init(void)
{
	return MV_OK;
}

int mv_ddr_pre_training_soc_config(const char *ddr_type)
{
	reg_write(0x116D8, 0x3CC);
#if defined(A80X0)
	reg_write(0x6F0100, 0x4480006);	/* DSS_CR0_REG_ADDR: define dimm configuration */
#endif
#if defined(A70X0)
	reg_write(0x6F0100, 0x44C0006);	/* DSS_CR0_REG_ADDR: define on-board configuration */
#endif
	reg_write(0x119D4, 0x2);	/* DRAM_PINS_MUX_REG: defines dimm or on-board, need to change in dimm */
	reg_write(DEV_GENERAL_CTRL38_REG, 0x6c6c0000);	/* vref calibration values */
	reg_write(0x6F4360, 0xFFFF0044);	/* ref range select */
	reg_write(0x11524, 0x8800);	/* DDR_IO_REG: data and control CMOS buffer and clk dram phy clk ration */

	return MV_OK;
}

int mv_ddr_post_training_soc_config(const char *ddr_type)
{
	/* set mux to MC6 */
	mmio_write_32(0xf00116d8, 0x38c);

	return MV_OK;
}

u32 mv_ddr_init_freq_get(void)
{
	enum hws_ddr_freq freq;

	mv_ddr_sar_freq_get(DEV_NUM_0, &freq);

	return freq;
}

void mv_ddr_mc_config(void)
{
	/* Memory controller initializations */
	struct init_cntr_param init_param;
	int status;

	init_param.do_mrs_phy = 1;
	init_param.is_ctrl64_bit = 1;
	init_param.init_phy = 1;
	init_param.msys_init = 1;

	status = hws_ddr3_tip_init_controller(0, &init_param);
	if (MV_OK != status)
		printf("mv_ddr %s: failed: err code 0x%x\n", __FUNCTION__, status);

	status = mv_ddr_mc6_init_controller();
	if (MV_OK != status)
		printf("mv_ddr %s: failed: err code 0x%x\n", __FUNCTION__, status);
}

/* FIXME: this is a place holder for the mc6 generic init currently hard coded */
int mv_ddr_mc6_init_controller(void)
{
	mv_ddr_mc6_and_dram_timing_set();

	mv_ddr_mc6_sizes_cfg();

	if (ddr3_if_ecc_enabled())
		mv_ddr_mc6_ecc_enable();

	reg_write(0x20064, 0x606);	/* MC "readReady"+ MC2PHYlatency */
	reg_write(0x21180, 0x500);	/* PHY_RL_Control for CS0:phy_rl_cycle_dly and phy_rl_tap_dly*/
	reg_write(0x21000, 0x60);	/* phy_rfifo_rptr_dly_val */
	reg_write(0x210c0, 0x81000001);	/* PHY_WL_RL_Control: bit 31 phy_rdq_sel_u_en for pod '1' phy_rl_enable = '1' */
	reg_write(0x202c8, 0xfefe);	/* MC_Control_3: phy_in_ff_bypass, phy_out_ff_bypass */
	reg_write(0x20340, 0x0);	/* ODT_Control_1: ODT_write_en, ODT0_read_en */
	reg_write(0x20344, 0x30000000);	/* ODT_Control_2: force_odt - always 'on' for cs0 and cs1*/
	reg_write(0x20310, 0x21000000);	/* DRAM_Config_5 CS0: RTT_Park, RTT_WR */
	reg_write(0x20318, 0x0);	/* DRAM_Config_5 CS2: RTT_Park, RTT_WR */
	reg_write(0x2031c, 0x0);	/* DRAM_Config_5 CS3: RTT_Park, RTT_WR */
	reg_write(0x20304, 0x0);	/* DRAM_Config_2 */
	reg_write(0x20308, 0x1);	/* DRAM_Config_3 DLL_reset */
#if defined(A80X0)
	reg_write(0x20314, 0x0);	/* DRAM_Config_5 CS1: RTT_Park, RTT_WR - Diff1 */
#if defined(CONFIG_64BIT)
	reg_write(0x20044, 0x30400);	/* MC_Control_0 - bust length, data width need to configure - diff4 - config */
#else
	reg_write(0x20044, 0x30300);	/* MC_Control_0 - bust length, data width need to configure - diff4 - config */
#endif	/* (CONFIG_64BIT) */
#endif	/* #if defined(A80X0) */
#if defined(A70X0)
	reg_write(0x20314, 0x21010000);
	reg_write(0x20044, 0x30300);
#endif	/* #if defined(A70X0) */
	reg_write(0x202c0, 0x6000);	/* MC_Control_1 - tw2r_dis? , acs_exit_dly timing???, config?? */
	reg_write(0x202c4, 0x120030);	/* MC_Control_2 - sdram typ, mode 2t, mirror en, rdimm mode - config */
	reg_write(0x20180, 0x30200);	/* RPP_Starvation_Control - default */
	reg_write(0x20050, 0xff);	/* Spool_Control default */
	reg_write(0x20054, 0x4c0);	/* MC_pwr_ctl - default */
	reg_write(0x2030c, 0x90000);	/* DRAM_Config_4: vref training value, odt? - config */

	return MV_OK;
}

#if defined(CONFIG_DDR4)
/* function: ddr4TipCalibrationValidate
 * this function validates the calibration values
 * the function is per soc due to the different processes the calibration values are different
 */
MV_STATUS mv_ddr4_calibration_validate(MV_U32 dev_num)
{
	MV_STATUS status = MV_OK;
	MV_U8 if_id = 0;
	MV_U32 read_data[MAX_INTERFACE_NUM];
	MV_U32 cal_n = 0, cal_p = 0;

	/*
	 * Pad calibration control enable: during training set the calibration to be internal
	 * at the end of the training it should be fixed to external to be configured by the mc6
	 * FIXME: set the calibration to external in the end of the training
	 */

	/* pad calibration control enable */
	CHECK_STATUS(ddr3_tip_if_write
			(DEV_NUM_0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
			DYN_PADS_CAL_ENABLE_ENA << DYN_PADS_CAL_ENABLE_OFFS |
			CAL_UPDATE_CTRL_INT << CAL_UPDATE_CTRL_OFFS,
			DYN_PADS_CAL_ENABLE_MASK << DYN_PADS_CAL_ENABLE_OFFS |
			CAL_UPDATE_CTRL_MASK << CAL_UPDATE_CTRL_OFFS));

	/* Polling initial calibration is done*/
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id,
				CAL_MACH_RDY << CAL_MACH_STATUS_OFFS,
				CAL_MACH_STATUS_MASK << CAL_MACH_STATUS_OFFS,
				MAIN_PADS_CAL_MACH_CTRL_REG, MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("ddr4TipCalibrationAdjust: DDR4 calibration poll failed(0)\n"));

	/* Polling that calibration propagate to io */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3FFFFFF, 0x3FFFFFF, PHY_LOCK_STATUS_REG,
				MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("ddr4TipCalibrationAdjust: DDR4 calibration poll failed(1)\n"));

	/* TODO - debug why polling not enough*/
	mdelay(10);

	/* pad calibration control disable */
	CHECK_STATUS(ddr3_tip_if_write
			(DEV_NUM_0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, MAIN_PADS_CAL_MACH_CTRL_REG,
			DYN_PADS_CAL_ENABLE_DIS << DYN_PADS_CAL_ENABLE_OFFS |
			CAL_UPDATE_CTRL_INT << CAL_UPDATE_CTRL_OFFS,
			DYN_PADS_CAL_ENABLE_MASK << DYN_PADS_CAL_ENABLE_OFFS |
			CAL_UPDATE_CTRL_MASK << CAL_UPDATE_CTRL_OFFS));

	/* Polling initial calibration is done */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id,
				CAL_MACH_RDY << CAL_MACH_STATUS_OFFS,
				CAL_MACH_STATUS_MASK << CAL_MACH_STATUS_OFFS,
				MAIN_PADS_CAL_MACH_CTRL_REG, MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("ddr4TipCalibrationAdjust: DDR4 calibration poll failed(0)\n"));

	/* Polling that calibration propagate to io */
	if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3FFFFFF, 0x3FFFFFF, PHY_LOCK_STATUS_REG,
				MAX_POLLING_ITERATIONS) != MV_OK)
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("ddr4TipCalibrationAdjust: DDR4 calibration poll failed(1)\n"));

	/* TODO - debug why polling not enough */
	mdelay(10);

	/* Read Cal value and set to manual val */
	CHECK_STATUS(ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1DC8, read_data, MASK_ALL_BITS));
	cal_n = (read_data[if_id] & ((0x3F) << 10)) >> 10;
	cal_p = (read_data[if_id] & ((0x3F) << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("ddr4TipCalibrationValidate::DDR4 SSTL calib val - Pcal = 0x%x , Ncal = 0x%x\n",
			   cal_p, cal_n));
	if ((cal_n >= 19) || (cal_n <= 2) || (cal_p >= 25) || (cal_p <= 4)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr: manual calibration\n"));
				/* ("ddr4TipCalibrationValidate: DDR4 SSTL calib val - Pcal = 0x%x , Ncal = 0x%x \
				   are out of range\n", cal_p, cal_n)); */
		status = MV_FAIL;
	}

	/* 14C8 - Horizontal TODO: check if this is horizontal, in 380 it is vertical */
	CHECK_STATUS(ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x14C8, read_data, MASK_ALL_BITS));
	cal_n = (read_data[if_id] & ((0x3F) << 10)) >> 10;
	cal_p = (read_data[if_id] & ((0x3F) << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("ddr4TipCalibrationValidate::DDR4 SSTL-H calib val - Pcal = 0x%x, \
					     Ncal = 0x%x \n", cal_p, cal_n));
	if ((cal_n >= 19) || (cal_n <= 2) || (cal_p >= 25) || (cal_p <= 4)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr: manual calibration\n"));
				/* ("ddr4TipCalibrationValidate: DDR4 SSTL-H calib val - Pcal = 0x%x, \
				   Ncal = 0x%x are out of range\n", cal_p, cal_n)); */
		status = MV_FAIL;
	}

	/* 17C8 - Horizontal */
	CHECK_STATUS(ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x17C8, read_data, MASK_ALL_BITS));
	cal_n = (read_data[if_id] & ((0x3F) << 10)) >> 10;
	cal_p = (read_data[if_id] & ((0x3F) << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("ddr4TipCalibrationValidate::DDR4 POD-H calib val - Pcal = 0x%x , Ncal = 0x%x\n",
			   cal_p, cal_n));
	if ((cal_n >= 32) || (cal_n <= 4) || (cal_p >= 17) || (cal_p <= 3)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr: manual calibration\n"));
				/* ("ddr4TipCalibrationValidate: DDR4 POD-H calib val - Pcal = 0x%x , \
				   Ncal = 0x%x are out of range\n", cal_p, cal_n)); */
		status = MV_FAIL;
	}

	/* 1EC8 - Vertical */
	CHECK_STATUS(ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1EC8, read_data, MASK_ALL_BITS));
	cal_n = (read_data[if_id] & ((0x3F) << 10)) >> 10;
	cal_p = (read_data[if_id] & ((0x3F) << 4)) >> 4;
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("ddr4TipCalibrationValidate::DDR4 POD-V calib val - Pcal = 0x%x , Ncal = 0x%x\n",
			   cal_p, cal_n));
	if ((cal_n >= 32) || (cal_n <= 4) || (cal_p >= 17) || (cal_p <= 3)) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("mv_ddr: manual calibration\n"));
				/* ("ddr4TipCalibrationValidate: DDR4 POD-V calib val - Pcal = 0x%x , \
				   Ncal = 0x%x are out of range\n", cal_p, cal_n)); */
		status = MV_FAIL;
	}

	/* FIXME: in case calibration failure set static calibration values, remove this in next release (A0 only) */
	if (status == MV_FAIL) {
		status = MV_OK;
#if defined(A70X0)
		reg_write(0x114CC, 0x1200D);
		reg_write(0x114C8, 0x1840008);
		reg_write(0x117C8, 0x28A0008);
		reg_write(0x11DC8, 0x1840008);
		reg_write(0x11EC8, 0x28A0008);
#endif
#if defined(A80X0)
		reg_write(0x114CC, 0x1200D);
		reg_write(0x114C8, 0x1840008);
		reg_write(0x117C8, 0x28A0008);
		reg_write(0x11DC8, 0x1840008);
		reg_write(0x11EC8, 0x28A0008);
#endif
	}

	return status;
}
#endif /* CONFIG_DDR4 */

/* function: mv_ddr_set_calib_controller
 * this function sets the controller which will control
 * the calibration cycle in the end of the training.
 * 1 - internal controller
 * 2 - external controller
 */
void mv_ddr_set_calib_controller(void)
{
	calibration_update_control = CAL_UPDATE_CTRL_EXT;
}

/* function: mv_ddr_mc_init
 * enables the controllers dunit and mc6 in all relevant cs
 */
int mv_ddr_mc_init(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	ddr3_tip_apn806_select_ddr_controller(DEV_NUM_0, TUNING_ACTIVE_SEL_TIP);

	/* enable dunit - the init is per cs and reference to 15e0 'DDR3_RANK_CTRL_REG' */
	ddr3_tip_if_write(DEV_NUM_0, ACCESS_TYPE_UNICAST, IF_ID_0, SDRAM_INIT_CTRL_REG, 0x1, 0x1);

	/* enable mc6 - the init is per cs in mc6 and not referenced */
	reg_write(MC6_USER_CMD0_REG, 0x10000001 |
		  ((tm->interface_params[0].as_bus_params[0].cs_bitmask <<
		    USER_CMD0_CS_OFFS) &
		   (USER_CMD0_CS_MASK << USER_CMD0_CS_OFFS)));

	/* FIXME: change the delay to polling on bit '0' */
	mdelay(10);

	return MV_OK;
}

int ddr3_tip_configure_phy(u32 dev_num)
{
	u32 cs_num;
	u32 vref;

	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_DATA, ADLL_CFG0_PHY_REG, 0x1503));
	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_DATA, ADLL_CFG1_PHY_REG, 0x50));
	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_DATA, ADLL_CFG2_PHY_REG, 0x12));
	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_CONTROL, ADLL_CFG0_PHY_REG, 0x1503));
	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_CONTROL, ADLL_CFG1_PHY_REG, 0x50));
	/* ADLL */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_CONTROL, ADLL_CFG2_PHY_REG, 0x12));
	/* data receiver calibration */
	CHECK_STATUS(ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, DDR_PHY_DATA, VREF_BCAST_PHY_REG(0), 0x20));
	/* data drive strength */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			   PARAM_NOT_CARE, DDR_PHY_DATA, PAD_ZRI_CAL_PHY_REG, 0x78f);
	/* ctrl drive strength */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			   PARAM_NOT_CARE, DDR_PHY_CONTROL, PAD_ZRI_CAL_PHY_REG, 0x78f);

	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			   PARAM_NOT_CARE, DDR_PHY_DATA, TEST_ADLL_PHY_REG, 0x1);
	/* set the phy register core_tx_data_samp_edge to posedge */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			   PARAM_NOT_CARE, DDR_PHY_DATA, PHY_CTRL_PHY_REG, 0x6002);
	calc_cs_num(0, 0, &cs_num);

#if defined(A70X0)
	vref = 0x423;
#else /* A80X0 */
	vref = 0x425;
#endif
	/* vref configuration */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
			   PARAM_NOT_CARE, DDR_PHY_DATA, PAD_CFG_PHY_REG, vref);

	/* data odt */
	if (cs_num == 1)
		ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				   PARAM_NOT_CARE, DDR_PHY_DATA, PAD_ODT_CAL_PHY_REG, 0x8 << 6);
	else /* dual cs */
		ddr3_tip_bus_write(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				   PARAM_NOT_CARE, DDR_PHY_DATA, PAD_ODT_CAL_PHY_REG, 0x4 << 6);

	return MV_OK;
}

/*
 * TODO: dq to pad mapping detection code to be relocated
 * to the generic part of mv_ddr code.
 */
#if defined(MV_DDR_DQ_MAPPING_DETECT)
static u32 mv_ddr_pad_to_dq_detect(u32 dev_num, u32 iface, u32 subphy, u32 pad)
{
	enum hws_training_ip_stat train_result[MAX_INTERFACE_NUM];
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 i, a, b, diff, max_diff, max_diff_cnt, dq;

	/*
	 * Note: HWS_LOW2HIGH direction didn't work because of asymmetry
	 * between tx windows (revealed by tap tuning function)
	 */
	enum hws_search_dir search_dir = HWS_HIGH2LOW;
	u8 prior_result[BUS_WIDTH_IN_BITS], post_result[BUS_WIDTH_IN_BITS];
	u32 *result[HWS_SEARCH_DIR_LIMIT];

	/* run training prior to any delay insertion */
	ddr3_tip_ip_training_wrapper(dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, RESULT_PER_BIT,
				     HWS_CONTROL_ELEMENT_ADLL,
				     PARAM_NOT_CARE, OPER_WRITE,
				     tm->if_act_mask, 0x0,
				     MAX_WINDOW_SIZE_TX - 1,
				     MAX_WINDOW_SIZE_TX - 1,
				     PATTERN_VREF, EDGE_FPF, CS_SINGLE,
				     PARAM_NOT_CARE, train_result);

	/* read training results */
	if (ddr3_tip_read_training_result(dev_num, iface,
					  ACCESS_TYPE_UNICAST, subphy,
					  ALL_BITS_PER_PUP, search_dir,
					  OPER_WRITE, RESULT_PER_BIT,
					  TRAINING_LOAD_OPERATION_UNLOAD,
					  CS_SINGLE, &result[search_dir],
					  1, 0, 0) != MV_OK)
		return MV_FAIL;

	/* save prior to delay insertion results */
	for (i = 0; i < BUS_WIDTH_IN_BITS; i++)
		prior_result[i] = result[search_dir][i] & 0xff;

	ddr3_hws_set_log_level(DEBUG_BLOCK_CENTRALIZATION, DEBUG_LEVEL_INFO);

#if MV_DDR_DQ_MAPPING_DETECT_VERBOSE == 1
	printf("MV_DDR: %s: Prior to DQ shift: if %d, subphy %d, pad %d,\n"
	       "\tregs: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
	       __func__, iface, subphy, pad,
	       prior_result[0], prior_result[1], prior_result[2], prior_result[3],
	       prior_result[4], prior_result[5], prior_result[6], prior_result[7]);
#endif

	/* insert delay to pad under test (max val is 0x1f) */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST,
			     iface, ACCESS_TYPE_UNICAST,
			     subphy, DDR_PHY_DATA,
			     PBS_TX_PHY_REG(0, pad), 0x1f);

	/* run training after delay insertion */
	ddr3_tip_ip_training_wrapper(dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, RESULT_PER_BIT,
				     HWS_CONTROL_ELEMENT_ADLL,
				     PARAM_NOT_CARE, OPER_WRITE,
				     tm->if_act_mask, 0x0,
				     MAX_WINDOW_SIZE_TX - 1,
				     MAX_WINDOW_SIZE_TX - 1,
				     PATTERN_VREF, EDGE_FPF, CS_SINGLE,
				     PARAM_NOT_CARE, train_result);

	/* read training results */
	if (ddr3_tip_read_training_result(dev_num, iface,
					  ACCESS_TYPE_UNICAST, subphy,
					  ALL_BITS_PER_PUP, search_dir,
					  OPER_WRITE, RESULT_PER_BIT,
					  TRAINING_LOAD_OPERATION_UNLOAD,
					  CS_SINGLE, &result[search_dir],
					  1, 0, 0) != MV_OK)
		return MV_FAIL;

	/* save post delay insertion results */
	for (i = 0; i < BUS_WIDTH_IN_BITS; i++)
		post_result[i] = result[search_dir][i] & 0xff;

#if MV_DDR_DQ_MAPPING_DETECT_VERBOSE == 1
	printf("MV_DDR: %s: After DQ shift: if %d, subphy %d, pad %d,\n"
	       "\tregs: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
	       __func__, iface, subphy, pad,
	       post_result[0], post_result[1], post_result[2], post_result[3],
	       post_result[4], post_result[5], post_result[6], post_result[7]);
#endif

	/* remove inserted to pad delay */
	ddr3_tip_bus_write(dev_num, ACCESS_TYPE_UNICAST,
			   iface, ACCESS_TYPE_UNICAST,
			   subphy, DDR_PHY_DATA,
			   PBS_TX_PHY_REG(0, pad), 0x0);

	/* find max diff and its occurrence num */
	max_diff = 0, max_diff_cnt = 0, dq = 0;
	for (i = 0; i < BUS_WIDTH_IN_BITS; i++) {
		a = prior_result[i];
		b = post_result[i];
		if (a > b)
			diff = a - b;
		else
			diff = 0; /* tx version */

		if (diff > max_diff) {
			max_diff = diff;
			dq = i;
			max_diff_cnt = 0;
		} else if (diff == max_diff) {
			max_diff_cnt++;
		}
	}
#if MV_DDR_DQ_MAPPING_DETECT_VERBOSE == 1
	printf("MV_DDR: %s: if %d, subphy %d, pad %d, max diff = %d, max diff count = %d, dq = %d\n",
	       __func__, iface, subphy, pad, max_diff, max_diff_cnt, dq);
#endif

	/* check for pad to dq pairing criteria */
	if (max_diff > 2 && max_diff_cnt == 0)
#if MV_DDR_DQ_MAPPING_DETECT_VERBOSE == 1
		printf("MV_DDR: %s: if %d, subphy %d, DQ[%d] = PAD[%d]\n",
		       __func__, iface, subphy, dq, pad);
#else
		;
#endif
	else
		dq = 0xff;

	return dq;
}

#define MV_DDR_DQ_MAPPING_DETECT_NTRIES 5

int mv_ddr_dq_mapping_detect(u32 dev_num)
{
	u32 iface, subphy, pad, dq_detected;
	int ntries;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM];
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	u32 mv_ddr_dq_mapping_detected[MAX_INTERFACE_NUM][MAX_BUS_NUM][BUS_WIDTH_IN_BITS] = {0};
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (iface = 0; iface < MAX_INTERFACE_NUM; iface++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, iface);
		/* save current cs enable reg val */
		ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST,
				 iface, DUAL_DUNIT_CFG_REG,
				 cs_enable_reg_val,
				 MASK_ALL_BITS);
		/* enable single cs */
		ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST,
				  iface, DUAL_DUNIT_CFG_REG,
				  (1 << 3), (1 << 3));

	}

	for (iface = 0; iface < MAX_INTERFACE_NUM; iface++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, iface);
		for (subphy = 0; subphy < octets_per_if_num; subphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy);
			for (pad = 0; pad < 11; pad++) {
				ntries = MV_DDR_DQ_MAPPING_DETECT_NTRIES;
				/*
				 * TODO: This part is platform-dependent.
				 * For APN806 platform: pad 3 is DM, pad 4 & 5 are DQS ones.
				 */
				if (pad == 3 || pad == 4 || pad == 5)
					continue;
				do {
					dq_detected = mv_ddr_pad_to_dq_detect(dev_num, iface, subphy, pad);
					ntries--;
				} while (dq_detected == 0xff && ntries > 0);

				if (dq_detected == 0xff)
					printf("MV_DDR: %s: Error: if %d, subphy %d, DQ for PAD[%d] not found after %d tries!\n",
					       __func__, iface, subphy, pad, MV_DDR_DQ_MAPPING_DETECT_NTRIES - ntries);
				else
					mv_ddr_dq_mapping_detected[iface][subphy][dq_detected] = pad;
			}
		}
	}

	/* restore cs enable value */
	for (iface = 0; iface < MAX_INTERFACE_NUM; iface++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, iface);
		ddr3_tip_if_write(dev_num, ACCESS_TYPE_UNICAST,
				  iface, DUAL_DUNIT_CFG_REG,
				  cs_enable_reg_val[iface],
				  MASK_ALL_BITS);
	}

	printf("MV_DDR: %s: dq to pad mapping detection results:\n", __func__);
	for (iface = 0; iface < MAX_INTERFACE_NUM; iface++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, iface);
		printf("if/subphy:\tdq0\tdq1\tdq2\tdq3\tdq4\tdq5\tdq6\tdq7\n");
		for (subphy = 0; subphy < octets_per_if_num; subphy++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, subphy);
			printf("%d/%d:\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", iface, subphy,
			       mv_ddr_dq_mapping_detected[iface][subphy][0],
			       mv_ddr_dq_mapping_detected[iface][subphy][1],
			       mv_ddr_dq_mapping_detected[iface][subphy][2],
			       mv_ddr_dq_mapping_detected[iface][subphy][3],
			       mv_ddr_dq_mapping_detected[iface][subphy][4],
			       mv_ddr_dq_mapping_detected[iface][subphy][5],
			       mv_ddr_dq_mapping_detected[iface][subphy][6],
			       mv_ddr_dq_mapping_detected[iface][subphy][7]);
		}
	}

	return MV_OK;
}

#endif

int apn806_rev_id_get(void)
{
	return (mmio_read_32(MVEBU_CSS_GWD_CTRL_IIDR2_REG) >> GWD_IIDR2_REV_ID_OFFSET) & GWD_IIDR2_REV_ID_MASK;
}
