/*
 * board.c
 *
 * Board functions for elin-w160 evaluation board, based on TI AM335x
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2014, u-blox AG - http://www.u-blox.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (C) 2014, connectBlue AB, http://www.connectblue.com/
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
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <power/tps65217.h>
#include "board.h"
DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_RMII1_INT		106
#define GPIO_RMII1_RESET    116
#define GPIO_RMII2_INT		82
#define GPIO_RMII2_RESET    115

#define LED_RED				100
#define LED_YELLOW			73
#define LED_GREEN			75
#define LED_BLUE			72
#define LED_ON				0
#define LED_OFF				1

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#if defined(CONFIG_SPL_BUILD)
static const struct ddr_data ddr3_data = {
    .datardsratio0 = MT41J128MJT125_RD_DQS,
    .datawdsratio0 = MT41J128MJT125_WR_DQS,
    .datafwsratio0 = MT41J128MJT125_PHY_FIFO_WE,
    .datawrsratio0 = MT41J128MJT125_PHY_WR_DATA,
    .datadldiff0 = PHY_DLL_LOCK_DIFF,
};

static const struct cmd_control ddr3_cmd_ctrl_data = {
    .cmd0csratio = MT41J128MJT125_RATIO,
    .cmd0dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
    .cmd0iclkout = MT41J128MJT125_INVERT_CLKOUT,

    .cmd1csratio = MT41J128MJT125_RATIO,
    .cmd1dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
    .cmd1iclkout = MT41J128MJT125_INVERT_CLKOUT,

    .cmd2csratio = MT41J128MJT125_RATIO,
    .cmd2dldiff = MT41J128MJT125_DLL_LOCK_DIFF,
    .cmd2iclkout = MT41J128MJT125_INVERT_CLKOUT,
};

static struct emif_regs ddr3_emif_reg_data = {
    .sdram_config = MT41J128MJT125_EMIF_SDCFG,
    .ref_ctrl = MT41J128MJT125_EMIF_SDREF,
    .sdram_tim1 = MT41J128MJT125_EMIF_TIM1,
    .sdram_tim2 = MT41J128MJT125_EMIF_TIM2,
    .sdram_tim3 = MT41J128MJT125_EMIF_TIM3,
    .zq_config = MT41J128MJT125_ZQ_CFG,
    .emif_ddr_phy_ctlr_1 = MT41J128MJT125_EMIF_READ_LATENCY |
        PHY_EN_DYN_PWRDN,
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
    debug("spl_start_uboot\n");
    /* break into full u-boot on 'c' */
    return (serial_tstc() && serial_getc() == 'c');
}
#endif

#define OSC	(V_OSCK/1000000)
const struct dpll_params dpll_ddr_elin_w160 = {
    400, OSC-1, 1, -1, -1, -1, -1};

/* Initialize LEDs on EVK */
void init_led(uint8_t pin, uint8_t status){
    gpio_request(pin,"");
    gpio_direction_output(pin, status);
}

void am33xx_spl_board_init(void)
{
    debug("ELIN-W160 SPL_BOARD_INIT\n");

    enable_i2c0_pin_mux();
    i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

    /* Get the frequency */
    dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);

    if (i2c_probe(TPS65217_CHIP_PM)){
        puts("Failed to probe PM chip\n");
        return;
    }else{
        debug("PM chip probed\n");
    }

    /* Set DCDC2 (MPU) voltage */
    if (tps65217_voltage_update(TPS65217_DEFDCDC2, TPS65217_DCDC_VOLT_SEL_1125MV)) {
        puts("tps65217_voltage_update failure\n");
        return;
    }else{
        debug("Set DCDC2 voltage\n");
    }

    /* Set DCDC3 (MPU) voltage */
    if (tps65217_voltage_update(TPS65217_DEFDCDC3, TPS65217_DCDC_VOLT_SEL_1125MV)) {
        puts("tps65217_voltage_update failure\n");
        return;
    }else{
        debug("Set DCDC3 voltage\n");
    }

    /* Set CORE Frequencies to OPP100 */
    do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

    /*
     * Set LDO3 to 1.8V and LDO4 to 3.3V.
     */
    if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
                TPS65217_DEFLS1,
                TPS65217_LDO_VOLTAGE_OUT_1_8,
                TPS65217_LDO_MASK)){
        puts("tps65217_reg_write failure\n");
    }else{
        debug("Set LDO3 voltage 1.8V\n");
    }

    if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
                TPS65217_DEFLS2,
                TPS65217_LDO_VOLTAGE_OUT_3_3,
                TPS65217_LDO_MASK)){
        puts("tps65217_reg_write failure\n");
    }else{
        debug("Set LDO4 voltage 3.3V\n");
    }
    /* Set MPU Frequency to what we detected now that voltages are set */
    do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
    debug("SPL_BOARD_INIT Complete\n");

    /* Ensure LED:s on EVK are off */
    init_led(LED_YELLOW, LED_OFF);
    init_led(LED_RED,    LED_OFF);
    init_led(LED_GREEN,  LED_OFF);
    init_led(LED_BLUE,   LED_OFF);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
    debug("get_dpll_ddr_params");
    return &dpll_ddr_elin_w160;
}

void set_uart_mux_conf(void)
{
#ifdef CONFIG_SERIAL1
    debug("UART0 init\n");
    enable_uart0_pin_mux();
#endif /* CONFIG_SERIAL1 */
}

void set_mux_conf_regs(void)
{
    debug("Set mux conf regs\n");
    enable_board_pin_mux();
}

void sdram_init(void)
{
    debug("SDRAM_INIT\n");
    config_ddr(400, MT41J128MJT125_IOCTRL_VALUE, &ddr3_data,
            &ddr3_cmd_ctrl_data, &ddr3_emif_reg_data, 0);
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
    debug("BOARD_INIT\n");
    gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

    gpmc_init();

    return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
    debug("BOARD_LATE_INIT\n");
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
    setenv("board_name", "ELIN_W160_EVB");
    setenv("board_rev", "1");
#endif

    return 0;
}
#endif

#if (defined(CONFIG_DRIVER_TI_CPSW) /*&& !defined(CONFIG_SPL_BUILD)*/) || \
    (defined(CONFIG_SPL_ETH_SUPPORT) && defined(CONFIG_SPL_BUILD)) 

static void cpsw_control(int enabled)
{
    /* VTP can be added here */
    return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
    {
        .slave_reg_ofs  = 0x208,
        .sliver_reg_ofs = 0xd80,
        .phy_id         = 3,
        .phy_if         = PHY_INTERFACE_MODE_RMII,
    },
    {
        .slave_reg_ofs  = 0x308,
        .sliver_reg_ofs = 0xdc0,
        .phy_id         = 0,
        .phy_if         = PHY_INTERFACE_MODE_RMII,
    },
};

static struct cpsw_platform_data cpsw_data = {
    .mdio_base      = CPSW_MDIO_BASE,
    .cpsw_base      = CPSW_BASE,
    .mdio_div       = 0xff,
    .channels       = 8,
    .cpdma_reg_ofs  = 0x800,
    .slaves         = 2,
    .slave_data     = cpsw_slaves,
    .ale_reg_ofs        = 0xd00,
    .ale_entries        = 1024,
    .host_port_reg_ofs  = 0x108,
    .hw_stats_reg_ofs   = 0x900,
    .bd_ram_ofs     = 0x2000,
    .mac_control        = (1 << 5),
    .control        = cpsw_control,
    .host_port_num      = 0,
    .version        = CPSW_CTRL_VERSION_2,
};

static void send_reset_pulse(int gpio_pin)
{
    gpio_request(gpio_pin,"");
    gpio_direction_output(gpio_pin, 1);
    udelay(1000);
    gpio_direction_output(gpio_pin, 0);
    udelay(1000);
    gpio_direction_output(gpio_pin, 1);
}
#endif

#if defined(CONFIG_DRIVER_TI_CPSW) || \
    (defined(CONFIG_USB_ETHER) && defined(CONFIG_MUSB_GADGET))
int board_eth_init(bd_t *bis)
{
    int rv, n = 0;
    uint8_t mac_addr[6];
    uint32_t mac_hi, mac_lo;
    send_reset_pulse(GPIO_RMII1_RESET);
    send_reset_pulse(GPIO_RMII2_RESET);

    /* try reading mac address from efuse */
    mac_lo = readl(&cdev->macid0l);
    mac_hi = readl(&cdev->macid0h);
    mac_addr[0] = mac_hi & 0xFF;
    mac_addr[1] = (mac_hi & 0xFF00) >> 8;
    mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
    mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
    mac_addr[4] = mac_lo & 0xFF;
    mac_addr[5] = (mac_lo & 0xFF00) >> 8;

    if (!getenv("ethaddr")) {
        printf("<ethaddr> not set. Validating first E-fuse MAC\n");

        if (is_valid_ether_addr(mac_addr))
            eth_setenv_enetaddr("ethaddr", mac_addr);
    }

	writel((RMII_MODE_ENABLE), &cdev->miisel);
    writel((GMII1_SEL_RMII | RMII1_IO_CLK_EN | GMII2_SEL_RMII | RMII2_IO_CLK_EN), &cdev->miisel);

    /* FIXME: Set SMA2 register (0x1320 offset) to use CRS_DV
	 * Required to ensure proper mux configuration see TRM */
    writel(0x01,0x44E11320);

    rv = cpsw_register(&cpsw_data);
    if (rv < 0)
        printf("Error %d registering CPSW switch\n", rv);
    else
        n += rv;

    return n;
}
#endif
