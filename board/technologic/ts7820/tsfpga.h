#ifndef __TSFPGA_H__
#define __TSFPGA_H__
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>

#define TS7820_SYSCON_BASE   (MBUS_PCI_MEM_BASE)
#define TS7820_SYSCON_SIZE   0x48

#define TS7820_FPGA_VENDOR   0x1172
#define TS7820_FPGA_DEVICE   0x0004

extern void __iomem *syscon_base;

#define fpga_peek32(offset) readl(syscon_base + (offset))
#define fpga_poke32(offset, value) writel(value, syscon_base + (offset))

/* Output Enables */
#define fpga_dio_oe_set(bank, value) fpga_poke32(0, value)
#define fpga_dio_oe_clr(bank, value) fpga_poke32(0 + 0x4, value)

/* Output Data */
#define fpga_dio_dat_set(bank, value) fpga_poke32(0 + 0x8, value)
#define fpga_dio_dat_clr(bank, value) fpga_poke32(0 + 0xc, value)

/* Output Data Readback */
#define fpga_dio_dat_out(bank) fpga_peek32(0 + 0x8)

/* Output Enable Data Readback */
#define fpga_dio_oe(bank) fpga_peek32(0 + 0x0)

/* Input Data */
#define fpga_dio_dat_in(bank) fpga_peek32(0 + 0xc)

#endif
