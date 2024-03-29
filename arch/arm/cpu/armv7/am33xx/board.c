/*
 * board.c
 *
 * Common board functions for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <asm/errno.h>
#include <linux/compiler.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/cache.h>

#define ARMV7_DCACHE_WRITEBACK  0xe
#define ARMV7_DOMAIN_CLIENT 1
#define ARMV7_DOMAIN_MASK (0x3 << 0)

DECLARE_GLOBAL_DATA_PTR;

static const struct gpio_bank gpio_bank_am33xx[4] = {
	{ (void *)AM33XX_GPIO0_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO1_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO2_BASE, METHOD_GPIO_24XX },
	{ (void *)AM33XX_GPIO3_BASE, METHOD_GPIO_24XX },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_am33xx;

#if defined(CONFIG_OMAP_HSMMC) && !defined(CONFIG_SPL_BUILD)
int cpu_mmc_init(bd_t *bis)
{
	int ret;

	ret = omap_mmc_init(0, 0, 0, -1, -1);
	if (ret)
		return ret;

	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif

/* AM33XX has two MUSB controllers which can be host or gadget */
#if (defined(CONFIG_MUSB_GADGET) || defined(CONFIG_MUSB_HOST)) && \
	(defined(CONFIG_AM335X_USB0) || defined(CONFIG_AM335X_USB1))
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* USB 2.0 PHY Control */
#define CM_PHY_PWRDN			(1 << 0)
#define CM_PHY_OTG_PWRDN		(1 << 1)
#define OTGVDET_EN			(1 << 19)
#define OTGSESSENDEN			(1 << 20)

static void am33xx_usb_set_phy_power(u8 on, u32 *reg_addr)
{
	if (on) {
		clrsetbits_le32(reg_addr, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN,
				OTGVDET_EN | OTGSESSENDEN);
	} else {
		clrsetbits_le32(reg_addr, 0, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	}
}

static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

#ifdef CONFIG_AM335X_USB0
static void am33xx_otg0_set_phy_power(u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl0);
}

struct omap_musb_board_data otg0_board_data = {
	.set_phy_power = am33xx_otg0_set_phy_power,
};

static struct musb_hdrc_platform_data otg0_plat = {
	.mode           = CONFIG_AM335X_USB0_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg0_board_data,
};
#endif

#ifdef CONFIG_AM335X_USB1
static void am33xx_otg1_set_phy_power(u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl1);
}

struct omap_musb_board_data otg1_board_data = {
	.set_phy_power = am33xx_otg1_set_phy_power,
};

static struct musb_hdrc_platform_data otg1_plat = {
	.mode           = CONFIG_AM335X_USB1_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg1_board_data,
};
#endif
#endif

int arch_misc_init(void)
{
#ifdef CONFIG_AM335X_USB0
	musb_register(&otg0_plat, &otg0_board_data,
		(void *)USB0_OTG_BASE);
#endif
#ifdef CONFIG_AM335X_USB1
	musb_register(&otg1_plat, &otg1_board_data,
		(void *)USB1_OTG_BASE);
#endif
	return 0;
}

#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_NOR_BOOT)
/*
 * This function is the place to do per-board things such as ramp up the
 * MPU clock frequency.
 */
__weak void am33xx_spl_board_init(void)
{
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

static void rtc32k_enable(void)
{
	struct rtc_regs *rtc = (struct rtc_regs *)RTC_BASE;

	/*
	 * Unlock the RTC's registers.  For more details please see the
	 * RTC_SS section of the TRM.  In order to unlock we need to
	 * write these specific values (keys) in this order.
	 */
	writel(0x83e70b13, &rtc->kick0r);
	writel(0x95a4f1e0, &rtc->kick1r);

	/* Enable the RTC 32K OSC by setting bits 3 and 6. */
	writel((1 << 3) | (1 << 6), &rtc->osc);
}

static void uart_soft_reset(void)
{
	struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;
	u32 regval;

	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_RESET;
	writel(regval, &uart_base->uartsyscfg);
	while ((readl(&uart_base->uartsyssts) &
		UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK)
		;

	/* Disable smart idle */
	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_SMART_IDLE_EN;
	writel(regval, &uart_base->uartsyscfg);
}

static void watchdog_disable(void)
{
	struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
}
#endif

void s_init(void)
{
	/*
	 * The ROM will only have set up sufficient pinmux to allow for the
	 * first 4KiB NOR to be read, we must finish doing what we know of
	 * the NOR mux in this space in order to continue.
	 */
#ifdef CONFIG_NOR_BOOT
	enable_norboot_pin_mux();
#endif
	/*
	 * Save the boot parameters passed from romcode.
	 * We cannot delay the saving further than this,
	 * to prevent overwrites.
	 */
#ifdef CONFIG_SPL_BUILD
	save_omap_boot_params();
#endif
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_NOR_BOOT)
	watchdog_disable();
	timer_init();
	set_uart_mux_conf();
	setup_clocks_for_console();
	uart_soft_reset();
#endif
#ifdef CONFIG_NOR_BOOT
	gd->baudrate = CONFIG_BAUDRATE;
	serial_init();
	gd->have_console = 1;
#else
	gd = &gdata;
	preloader_console_init();
#endif
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_NOR_BOOT)
	prcm_init();
	set_mux_conf_regs();
	/* Enable RTC32K clock */
	rtc32k_enable();
	sdram_init();
#endif
}
#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
    /* Enable D-cache. I-cache is already enabled in start.S */
    dcache_enable();
}

void dram_bank_mmu_setup(int bank)
{
    bd_t *bd = gd->bd;
    int i;

    u32 start = bd->bi_dram[bank].start >> 20;
    u32 size = bd->bi_dram[bank].size >> 20;
    u32 end = start + size;

    debug("%s: bank: %d\n", __func__, bank);
    for (i = start; i < end; i++)
        set_section_dcache(i, ARMV7_DCACHE_WRITEBACK);

}

void arm_init_domains(void)
{
    u32 reg;

    reg = get_dacr();
    /*
     * Set DOMAIN to client access so that all permissions
     * set in pagetables are validated by the mmu.
     */
    reg &= ~ARMV7_DOMAIN_MASK;
    reg |= ARMV7_DOMAIN_CLIENT;
    set_dacr(reg);
}
#endif
