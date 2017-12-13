#ifndef __TSFPGA_H__
#define __TSFPGA_H__
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>

#define TS7800_V2_SYSCON_BASE   (MBUS_PCI_MEM_BASE + 0x100000)
#define TS7800_V2_SYSCON_SIZE   0x48

#define fpga_peek32(offset) readl(TS7800_V2_SYSCON_BASE + offset)
#define fpga_poke32(offset, value) writel(value, TS7800_V2_SYSCON_BASE + offset)

#endif
