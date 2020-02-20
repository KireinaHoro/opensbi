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
#include <sbi_utils/serial/uart8250.h>
#include <sbi_utils/sys/clint.h>

/* clang-format off */

#define EDGEBOARD_HART_COUNT			1
#define EDGEBOARD_HART_STACK_SIZE			8192

#define EDGEBOARD_SYS_CLK				100000000

#define EDGEBOARD_CLINT_ADDR			0x2000000

#define EDGEBOARD_PLIC_ADDR				0xc000000
#define EDGEBOARD_PLIC_NUM_SOURCES			3
#define EDGEBOARD_PLIC_NUM_PRIORITIES		7

#define EDGEBOARD_UART0_ADDR			0xe0001000  // shift 0x1000 from Vivado address map
#define EDGEBOARD_UART_BAUDRATE			115200

#ifndef EDGEBOARD_ENABLED_HART_MASK
#define EDGEBOARD_ENABLED_HART_MASK	(1 << 0)
#endif

// Buffer for device tree modification
#define EDGEBOARD_FDT_RWBUF ((void*)0x58000)
#define EDGEBOARD_FDT_BUFSIZE 0x8000

#define EDGEBOARD_HARITD_DISABLED			~(EDGEBOARD_ENABLED_HART_MASK)

/* clang-format on */

static void edgeboard_modify_dt(const void *fdt, void *rwbuf, size_t bufsize)
{
	u32 i;
	int chosen_offset, err;
	int cpu_offset;
	char cpu_node[32] = "";
	const char *mmu_type;

	err  = fdt_open_into(fdt, rwbuf, bufsize);
	if (err < 0)
		sbi_printf(
			"Device Tree @ 0x%p can't be expanded into 0x%p: err %d\n",
			fdt, rwbuf, err);

	for (i = 0; i < EDGEBOARD_HART_COUNT; i++) {
		sbi_sprintf(cpu_node, "/cpus/cpu@%d", i);
		cpu_offset = fdt_path_offset(rwbuf, cpu_node);
		mmu_type   = fdt_getprop(rwbuf, cpu_offset, "mmu-type", NULL);
		if (mmu_type && (!strcmp(mmu_type, "riscv,sv39") ||
				 !strcmp(mmu_type, "riscv,sv48")))
			continue;
		else
			fdt_setprop_string(rwbuf, cpu_offset, "status",
					   "disabled");
		memset(cpu_node, 0, sizeof(cpu_node));
	}

	chosen_offset = fdt_path_offset(rwbuf, "/chosen");
	fdt_setprop_string(rwbuf, chosen_offset, "stdout-path",
			   "/soc/serial@e0000000:115200");

	plic_fdt_fixup(rwbuf, "riscv,plic0");
	
	sbi_printf("Modified device tree at 0x%p\n", rwbuf);
	struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();
	scratch->next_arg1 = (uint64_t)rwbuf;
}

static int edgeboard_final_init(bool cold_boot)
{
	void *fdt;

	if (!cold_boot)
		return 0;

	fdt = sbi_scratch_thishart_arg1_ptr();
	edgeboard_modify_dt(fdt, EDGEBOARD_FDT_RWBUF, EDGEBOARD_FDT_BUFSIZE);

	return 0;
}

static u32 edgeboard_pmp_region_count(u32 hartid)
{
	return 8;
}

static int edgeboard_pmp_region_info(u32 hartid, u32 index, ulong *prot,
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

static int edgeboard_console_init(void)
{
	unsigned long peri_in_freq = EDGEBOARD_SYS_CLK;

	return uart8250_init(EDGEBOARD_UART0_ADDR, peri_in_freq,
				EDGEBOARD_UART_BAUDRATE, 2, 4);
}

static int edgeboard_irqchip_init(bool cold_boot)
{
	int rc;
	u32 hartid = sbi_current_hartid();

	if (cold_boot) {
		rc = plic_cold_irqchip_init(EDGEBOARD_PLIC_ADDR,
					    EDGEBOARD_PLIC_NUM_SOURCES,
					    EDGEBOARD_HART_COUNT);
		if (rc)
			return rc;
	}

	return plic_warm_irqchip_init(hartid, (hartid) ? (2 * hartid - 1) : 0,
				      (hartid) ? (2 * hartid) : -1);
}

static int edgeboard_ipi_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_ipi_init(EDGEBOARD_CLINT_ADDR, EDGEBOARD_HART_COUNT);
		if (rc)
			return rc;
	}

	return clint_warm_ipi_init();
}

static int edgeboard_timer_init(bool cold_boot)
{
	int rc;

	if (cold_boot) {
		rc = clint_cold_timer_init(EDGEBOARD_CLINT_ADDR, EDGEBOARD_HART_COUNT, TRUE);
		if (rc)
			return rc;
	}

	return clint_warm_timer_init();
}

static int edgeboard_system_down(u32 type)
{
	/* For now nothing to do. */
	return 0;
}

const struct sbi_platform_operations platform_ops = {
	.pmp_region_count	= edgeboard_pmp_region_count,
	.pmp_region_info	= edgeboard_pmp_region_info,
	.final_init		= edgeboard_final_init,
	.console_putc		= uart8250_putc,
	.console_getc		= uart8250_getc,
	.console_init		= edgeboard_console_init,
	.irqchip_init		= edgeboard_irqchip_init,
	.ipi_send		= clint_ipi_send,
	.ipi_clear		= clint_ipi_clear,
	.ipi_init		= edgeboard_ipi_init,
	.timer_value		= clint_timer_value,
	.timer_event_stop	= clint_timer_event_stop,
	.timer_event_start	= clint_timer_event_start,
	.timer_init		= edgeboard_timer_init,
	.system_reboot		= edgeboard_system_down,
	.system_shutdown	= edgeboard_system_down
};

const struct sbi_platform platform = {
	.opensbi_version	= OPENSBI_VERSION,
	.platform_version	= SBI_PLATFORM_VERSION(0x0, 0x01),
	.name			= "ALINX EdgeBoard XAZU3EG",
	.features		= SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count		= EDGEBOARD_HART_COUNT,
	.hart_stack_size	= EDGEBOARD_HART_STACK_SIZE,
	.disabled_hart_mask	= EDGEBOARD_HARITD_DISABLED,
	.platform_ops_addr	= (unsigned long)&platform_ops
};
