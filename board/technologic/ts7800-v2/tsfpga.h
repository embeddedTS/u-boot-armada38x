#ifndef __TSFPGA_H__
#define __TSFPGA_H__
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>

#define TS7800_V2_SYSCON_BASE   ((volatile unsigned char*)(MBUS_PCI_MEM_BASE + 0x100000))
#define TS7800_V2_SYSCON_SIZE   0x48

#define TS7800_V2_FPGA_VENDOR   0x1204
#define TS7800_V2_FPGA_DEVICE   0x0001

extern volatile unsigned char *syscon_base;
#define fpga_peek32(offset) readl(syscon_base + (offset))
#define fpga_poke32(offset, value) writel(value, syscon_base + (offset))

#endif
