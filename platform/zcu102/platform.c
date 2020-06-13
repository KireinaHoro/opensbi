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

#define ZCU102_HART_COUNT			1
#define ZCU102_HART_STACK_SIZE			8192

#define ZCU102_SYS_CLK				100000000

#define ZCU102_CLINT_ADDR			0x2000000

#define ZCU102_PLIC_ADDR			0xc000000
#define ZCU102_PLIC_NUM_SOURCES			2
#define ZCU102_PLIC_NUM_PRIORITIES		3

#define ZCU102_UART0_ADDR			0x10020000
#define ZCU102_UART_BAUDRATE			115200

#ifndef ZCU102_ENABLED_HART_MASK
#define ZCU102_ENABLED_HART_MASK	(1 << 0)
#endif

#define ZCU102_HARITD_DISABLED			~(ZCU102_ENABLED_HART_MASK)

/* clang-format on */

static int zcu102_final_init(bool cold_boot)
{
	if (!cold_boot)
		return 0;

	return 0;
}

static u32 zcu102_pmp_region_count(u32 hartid)
{
	return 8;
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
		rc = clint_cold_timer_init(ZCU102_CLINT_ADDR, ZCU102_HART_COUNT, TRUE);
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
	.name			= "Xilinx ZCU102 Rocket Chip",
	.features		= SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count		= ZCU102_HART_COUNT,
	.hart_stack_size	= ZCU102_HART_STACK_SIZE,
	.disabled_hart_mask	= ZCU102_HARITD_DISABLED,
	.platform_ops_addr	= (unsigned long)&platform_ops
};
