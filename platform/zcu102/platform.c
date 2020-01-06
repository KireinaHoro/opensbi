/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

#include <libfdt.h>
#include <fdt.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_platform.h>
#include <sbi/riscv_io.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/sifive-uart.h>
#include <sbi_utils/sys/clint.h>

/* clang-format off */

#define ZCU102_HART_COUNT			4
#define ZCU102_HART_STACK_SIZE			8192

#define ZCU102_SYS_CLK				100000000

#define ZCU102_CLINT_ADDR			0x2000000

#define ZCU102_PLIC_ADDR				0xc000000
#define ZCU102_PLIC_NUM_SOURCES			6
#define ZCU102_PLIC_NUM_PRIORITIES		7

#define ZCU102_UART0_ADDR			0x60401000  // shift 0x1000 from Vivado address map
#define ZCU102_UART_BAUDRATE			115200

#ifndef ZCU102_ENABLED_HART_MASK
#define ZCU102_ENABLED_HART_MASK	(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3)
#endif

#define ZCU102_HARITD_DISABLED			~(ZCU102_ENABLED_HART_MASK)

/* clang-format on */

static void zcu102_modify_dt(void *fdt)
{
	u32 i, size;
	int chosen_offset, err;
	int cpu_offset;
	char cpu_node[32] = "";
	const char *mmu_type;

	size = fdt_totalsize(fdt);
	err  = fdt_open_into(fdt, fdt, size + 256);
	if (err < 0)
		sbi_printf(
			"Device Tree can't be expanded to accmodate new node");

	for (i = 0; i < ZCU102_HART_COUNT; i++) {
		sbi_sprintf(cpu_node, "/cpus/cpu@%d", i);
		cpu_offset = fdt_path_offset(fdt, cpu_node);
		mmu_type   = fdt_getprop(fdt, cpu_offset, "mmu-type", NULL);
		if (mmu_type && (!strcmp(mmu_type, "riscv,sv39") ||
				 !strcmp(mmu_type, "riscv,sv48")))
			continue;
		else
			fdt_setprop_string(fdt, cpu_offset, "status",
					   "disabled");
		memset(cpu_node, 0, sizeof(cpu_node));
	}

	chosen_offset = fdt_path_offset(fdt, "/chosen");
	fdt_setprop_string(fdt, chosen_offset, "stdout-path",
			   "/soc/serial@60400000:115200");

	plic_fdt_fixup(fdt, "riscv,plic0");
}

static int zcu102_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();
	zcu102_modify_dt(fdt);

	return 0;
}

static u32 zcu102_pmp_region_count(u32 hartid)
{
	return 1;
}

static int zcu102_pmp_region_info(u32 hartid, u32 index, ulong *prot,
				 ulong *addr, ulong *log2size)
{
	int ret = 0;

	switch (index) {
	case 0:
		*prot	  = PMP_R | PMP_W | PMP_X;
		*addr	  = 0;
		*log2size = __riscv_xlen;
		break;
	default:
		ret = -1;
		break;
	};

	return ret;
}

static int zcu102_console_init(void)
{
	unsigned long peri_in_freq = ZCU102_SYS_CLK;

	return sifive_uart_init(ZCU102_UART0_ADDR, peri_in_freq,
				ZCU102_UART_BAUDRATE);
}

static int zcu102_irqchip_init(bool cold_boot)
{
	int rc;
	u32 hartid = sbi_current_hartid();

	if (cold_boot) {
		rc = plic_cold_irqchip_init(ZCU102_PLIC_ADDR,
					    ZCU102_PLIC_NUM_SOURCES,
					    ZCU102_HART_COUNT);
		if (rc)
			return rc;
	}

	return plic_warm_irqchip_init(hartid, (hartid) ? (2 * hartid - 1) : 0,
				      (hartid) ? (2 * hartid) : -1);
}

static int zcu102_ipi_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_ipi_init(ZCU102_CLINT_ADDR, ZCU102_HART_COUNT);
		if (rc)
			return rc;
	}

	return clint_warm_ipi_init();
}

static int zcu102_timer_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_timer_init(ZCU102_CLINT_ADDR, ZCU102_HART_COUNT);
		if (rc)
			return rc;
	}

	return clint_warm_timer_init();
}

static int zcu102_system_down(u32 type)
{
	/* For now nothing to do. */
	return 0;
}

const struct sbi_platform_operations platform_ops = {
	.pmp_region_count	= zcu102_pmp_region_count,
	.pmp_region_info	= zcu102_pmp_region_info,
	.final_init		= zcu102_final_init,
	.console_putc		= sifive_uart_putc,
	.console_getc		= sifive_uart_getc,
	.console_init		= zcu102_console_init,
	.irqchip_init		= zcu102_irqchip_init,
	.ipi_send		= clint_ipi_send,
	.ipi_clear		= clint_ipi_clear,
	.ipi_init		= zcu102_ipi_init,
	.timer_value		= clint_timer_value,
	.timer_event_stop	= clint_timer_event_stop,
	.timer_event_start	= clint_timer_event_start,
	.timer_init		= zcu102_timer_init,
	.system_reboot		= zcu102_system_down,
	.system_shutdown	= zcu102_system_down
};

const struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	= SBI_PLATFORM_VERSION(0x0, 0x01),
	.name			= "Xilinx Zynq UltraScale+ ZCU102",
	.features		= SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count		= ZCU102_HART_COUNT,
	.hart_stack_size	= ZCU102_HART_STACK_SIZE,
	.disabled_hart_mask	= ZCU102_HARITD_DISABLED,
	.platform_ops_addr	= (unsigned long)&platform_ops
};
