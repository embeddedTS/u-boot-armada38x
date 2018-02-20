#ifndef __TS78FPGA_H__
#define __TS78FPGA_H__
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>

#define TS7840_SYSCON_BASE   ((volatile unsigned char*)(MBUS_PCI_MEM_BASE + 0x100000))
#define TS7840_SYSCON_SIZE   0x48
#define TS7840_FPGA_VENDOR   0x1172
#define TS7840_FPGA_DEVICE   0x0004

extern volatile unsigned char *syscon_base;
#define fpga_peek32(offset) readl(syscon_base + (offset))
#define fpga_poke32(offset, value) writel(value, syscon_base + (offset))

#endif
