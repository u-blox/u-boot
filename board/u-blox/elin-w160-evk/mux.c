/*
 * mux.c
 *
 * Pin mux functions for elin-w160 evaluation board
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2014 u-blox AG - http://www.u-blox.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD0 */
	{OFFSET(gpmc_ad1), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD1 */
	{OFFSET(gpmc_ad2), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD2 */
	{OFFSET(gpmc_ad3), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD3 */
	{OFFSET(gpmc_ad4), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD4 */
	{OFFSET(gpmc_ad5), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD5 */
	{OFFSET(gpmc_ad6), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD6 */
	{OFFSET(gpmc_ad7), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* NAND AD7 */
	{OFFSET(gpmc_wait0), (MODE(0) | RXACTIVE | PULLUP_EN)}, /* NAND WAIT */
	{OFFSET(gpmc_wpn), (MODE(7) | PULLUP_EN | RXACTIVE)},	/* NAND_WPN */
	{OFFSET(gpmc_csn0), (MODE(0) | PULLUDEN)},				/* NAND_CS0 */
	{OFFSET(gpmc_advn_ale), (MODE(0) | PULLUDEN)},			/* NAND_ADV_ALE */
	{OFFSET(gpmc_oen_ren), (MODE(0) | PULLUDEN)},			/* NAND_OE */
	{OFFSET(gpmc_wen), (MODE(0) | PULLUDEN)},				/* NAND_WEN */
	{OFFSET(gpmc_be0n_cle), (MODE(0) | PULLUDEN)},			/* NAND_BE_CLE */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(mmc0_clk),  (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(mmc0_cmd),  (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{-1},
};


static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad8), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_ad9), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad10), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad11), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE | PULLUP_EN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE | PULLUP_EN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux led_pins_mux[] = {
	{OFFSET(lcd_data2), (MODE(7) )}, /* lcddata_2.gpio2.8 GPIO.IO6 */
	{OFFSET(lcd_data3), (MODE(7) )}, /* lcddata_3.gpio2.9 GPIO.IO5 */
	{OFFSET(lcd_data5), (MODE(7) )}, /* lcddata_5.gpio2.11 GPIO.IO7 */
	{OFFSET(mii1_rxdv), (MODE(7) )}, /* gmii_rxdv.gpio3.4 GPIO.IO10 */
	{-1},
};

static struct module_pin_mux rmii1_pin_mux[] = {
	{OFFSET(mii1_txen), MODE(1)},			    /* RMII1_TXEN */
	{OFFSET(mii1_txd1), MODE(1)},			    /* RMII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(1)},			    /* RMII1_TXD0 */
	{OFFSET(mii1_rxd1), MODE(1) | RXACTIVE},	/* RMII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(1) | RXACTIVE},	/* RMII1_RXD0 */
	{OFFSET(mii1_crs), MODE(1) | RXACTIVE},	    /* RMII1_CRS_DV */
	{OFFSET(mii1_rxerr), MODE(1) | RXACTIVE},   /* RMII1_RXER */
	{OFFSET(rmii1_refclk), MODE(0) | RXACTIVE}, /* RMII1_REFCLK */
	{OFFSET(mcasp0_axr1), MODE(7)},             /* RMII1_RESETN (emu3_mux2)*/
	{OFFSET(mii1_rxclk), MODE(7) | RXACTIVE | PULLUP_EN}, /* RMII1_POWERDOWN_INT_N */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{-1},
};

static struct module_pin_mux rmii2_pin_mux[] = {
	{OFFSET(gpmc_a0), MODE(3)},			        /* RMII2_TXEN */
	{OFFSET(gpmc_a4), MODE(3)},			        /* RMII2_TXD1 */
	{OFFSET(gpmc_a5), MODE(3)},			        /* RMII2_TXD0 */
	{OFFSET(mii1_col), MODE(1) | RXACTIVE},	    /* RMII2_REFCLK */
	{OFFSET(gpmc_a10), MODE(3) | RXACTIVE},	    /* RMII2_RXD1 */
	{OFFSET(gpmc_a11), MODE(3) | RXACTIVE},	    /* RMII2_RXD0 */
	{OFFSET(gpmc_a9), MODE(3) | RXACTIVE},      /* RMII2_CRS_DV */
	{OFFSET(mii1_rxd3), MODE(7) | RXACTIVE | PULLUP_EN},  /* RMII2_POWERDOWN_INT_N */
	{OFFSET(mcasp0_fsr), MODE(7)},              /* RMII2_RESETN */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}


void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_board_pin_mux(void)
{
	/* elin-w160 evaluation board pinmux */
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(nand_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(rmii1_pin_mux);
	configure_module_pin_mux(rmii2_pin_mux);
	configure_module_pin_mux(led_pins_mux);
}
