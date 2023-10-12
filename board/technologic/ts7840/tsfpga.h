#ifndef __TSFPGA_H__
#define __TSFPGA_H__
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>

#define TS_FPGA_VENDOR		0x1e6d
#define TS7840_FPGA_DEVICE	0x7840

#define FPGA_MODEL		0x0
#define FPGA_ASMI		0x8
#define FPGA_FRAM_BOOTCOUNT	0x18
#define FPGA_RELOAD		0x1a

extern void __iomem *syscon_base;

#define fpga_peek32(offset) readl(syscon_base + (offset))
#define fpga_poke32(offset, value) writel(value, syscon_base + (offset))

/* Output Enables */
#define fpga_dio_oe_set(bank, value) \
	fpga_poke32(0x24 + (bank * 0x1c) + 0x00, value)
#define fpga_dio_oe_clr(bank, value) \
	fpga_poke32(0x24 + (bank * 0x1c) + 0x04, value)

/* Output Data */
#define fpga_dio_dat_set(bank, value) \
	fpga_poke32(0x24 + (bank * 0x1c) + 0x08, value)
#define fpga_dio_dat_clr(bank, value) \
	fpga_poke32(0x24 + (bank * 0x1c) + 0x0c, value)

/* Output Data Readback */
#define fpga_dio_dat_out(bank) \
	fpga_peek32(0x24 + (bank * 0x1c) + 0x08)

/* Output Enable Data Readback */
#define fpga_dio_oe(bank) \
	fpga_peek32(0x24 + (bank * 0x1c) + 0x00)

/* Input Data */
#define fpga_dio_dat_in(bank) \
	fpga_peek32(0x24 + (bank * 0x1c) + 0x0c)

#endif
