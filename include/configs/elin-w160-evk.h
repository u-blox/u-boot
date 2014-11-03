/*
 * elin-w160-evk.h
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
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_ELIN_W160_EVK_H
#define __CONFIG_ELIN_W160_EVK_H

#include <configs/ti_am335x_common.h>

#define MACH_TYPE_TIAM335EVM		3589	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_TIAM335EVM

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Custom script for NOR */
#define CONFIG_SYS_LDSCRIPT		"board/u-blox/elin-w160-evk/u-boot.lds"

#ifndef CONFIG_SPL_BUILD

#define CONFIG_EXTRA_ENV_SETTINGS \
    "bootdelay=1\0" \
	"bootdir=/boot\0" \
	"bootcmd=mtdparts default; run nandboot;\0"\
	"console=ttyO0,115200n8\0"\
	"dfu_alt_info=" DFU_ALT_INFO_NAND "\0" \
	"dfu_upgrade=mtdparts default; dfu nand 0;\0"\
	"erase_environment=nand erase.part bootloader-env1;\0"\
	"fdtaddr=0x80F80000\0" \
	"fdt_high=0xffffffff\0" \
    "flash_fdt=nand erase.part fdt; nand write ${ramaddr} fdt\0"\
    "flash_kernel=nand erase.part kernel; nand write ${ramaddr} kernel\0"\
    "flash_mlo=nand erase.part spl1; nand write ${ramaddr} spl1\0"\
    "flash_bootloader=nand erase.part bootloader; nand write ${ramaddr} bootloader\0"\
    "flash_rootfs=nand erase.part rootfs;nand write ${ramaddr} rootfs ${filesize}\0"\
	"loadaddr=0x80200000\0"\
    "kloadaddr=0x80008000\0"\
    "mmcargs=setenv bootargs console=${console} ${optargs} root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait\0"\
    "mmcboot=echo Booting from MMC...; run mmcload; run mmcargs; bootz ${kloadaddr} - ${fdtaddr}\0"\
    "mmcload=load mmc 1:2 ${kloadaddr} ${bootdir}/kernel.img; load mmc 1:2 ${fdtaddr} ${bootdir}/fdt.img\0"\
    "mmc_update_bootloader=load mmc 1:2 ${ramaddr} ${bootdir}/u-boot.img; run flash_bootloader;\0"\
    "mmc_update_bootloader_mlo=load mmc 1:2 ${ramaddr} ${bootdir}/MLO; run flash_mlo;\0"\
    "mmc_update_fdt=load mmc 1:2 ${ramaddr} ${bootdir}/fdt.img; run flash_fdt;\0"\
    "mmc_update_kernel=load mmc 1:2 ${ramaddr} ${bootdir}/kernel.img; run flash_kernel;\0"\
    "mmc_update=mmc dev 1; run mmc_update_kernel; run mmc_update_fdt; run mmc_update_bootloader_mlo; run mmc_update_bootloader;run erase_environment;\0"\
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} "\
		"${optargs} "\
		"root=${nandroot} "\
		"rootfstype=${nandrootfstype} " \
		"${mtdparts}\0" \
    "nandboot=echo Booting from nand ...;"\
        "run nandargs; "\
        "nand read ${kloadaddr} kernel; "\
        "nand read ${fdtaddr} fdt;"\
        "bootz ${kloadaddr} - ${fdtaddr}\0"\
	"nandroot=ubi0:rootfs rw ubi.mtd=9,2048\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
    "netboot=mtdparts default; setenv autoload no;"\
		"bootp; if tftp ${loadaddr} /rootfs/boot/ubootscript.scr; then"\
		" source ${loadaddr}; else run nandboot; fi;\0"\
    "ramaddr=0x82000000\0"\
    "rdaddr=0x81000000\0"\

#define CONFIG_BOOTCOMMAND      "run bootcmd;"

#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* BASE EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */
#define CONFIG_BAUDRATE				115200
#define CONFIG_SYS_BAUDRATE_TABLE { 115200, 230400 }

/* PMIC support */
#define CONFIG_POWER_TPS65217

/* Add support for Flattened Device Tree */
#define CONFIG_OF_LIBFDT

/* SPL */
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_YMODEM_SUPPORT
#undef CONFIG_SPL_OS_BOOT

/* NAND support */
#define CONFIG_CMD_NAND

/* NAND Configuration */
#define CONFIG_CMD_NAND_LOCK_UNLOCK
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define GPMC_NAND_ECC_LP_x16_LAYOUT	1

/* NAND partition layout */
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	    0x80000  /* U-Boot starts here */
#define CONFIG_ENV_OFFSET		        0x180000 /* environment starts here */
#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:128k(spl1)," \
					"128k(spl2)," \
					"128k(spl3)," \
					"128k(spl4)," \
					"1024k(bootloader)," \
					"128k(bootloader-env1)," \
					"128k(bootloader-env2)," \
					"256k(fdt)," \
					"5120k(kernel)," \
					"441m(rootfs)," \
					"64m(rescue)"

/* Environment settings */
#define CONFIG_ENV_SIZE			(128 << 10)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */

/*
* USB configuration. We enable MUSB support, both for host and for
* gadget. We set USB0 as peripheral and USB1 as host, based on the
* board schematic and physical port wired to each. Then for host we
* add mass storage support and for gadget we add both RNDIS ethernet
* and DFU.
*/
#define CONFIG_SPL_MUSB_NEW_SUPPORT
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_MUSB_GADGET
#define CONFIG_MUSB_PIO_ONLY
#define CONFIG_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_USB_GADGET
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW 0
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE MUSB_PERIPHERAL
#define CONFIG_G_DNL_VENDOR_NUM 0x1546
#define CONFIG_G_DNL_PRODUCT_NUM 0x2000
#define CONFIG_G_DNL_MANUFACTURER "u-blox AG"


/* USB Device Firmware Update support */
#define CONFIG_DFU_FUNCTION
#define CONFIG_CMD_DFU

#ifdef CONFIG_NAND
#define CONFIG_DFU_NAND
#define CONFIG_SYS_DFU_DATA_BUF_SIZE (128*1024)
#define DFU_ALT_INFO_NAND \
	"spl1 part 0 1;" \
	"spl2 part 0 2;" \
	"spl3 part 0 3;" \
	"spl4 part 0 4;" \
	"bootloader part 0 5;" \
	"bootloader-env1 part 0 6;" \
	"bootloader-env2 part 0 7;" \
	"fdt part 0 8;" \
	"kernel part 0 9;" \
	"rootfs part 0 10;" \
	"rescue part 0 11"
#endif

/* General network SPL, both CPSW and USB gadget RNDIS */
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_USBETH_SUPPORT)
	/*Disable CPSW SPL support so we fit within the 101KiB limit.  */
	#undef CONFIG_SPL_ETH_SUPPORT
#endif

#define CONFIG_SPL_ETH_SUPPORT
#define CONFIG_SPL_NET_SUPPORT
#define CONFIG_PHY_ADDR 0x03
#define CONFIG_SPL_ENV_SUPPORT
#define CONFIG_SPL_NET_VCI_STRING	"AM335x U-Boot SPL"
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

/* Ethernet NET */
#define CONFIG_DRIVER_TI_CPSW
#define CONFIG_CMD_MDIO
#define CONFIG_CMD_MII
#define CONFIG_MII
#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL

#endif	/* ! __CONFIG_ELIN_W160_EVK_H */
