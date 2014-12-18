/*
 * board.h
 *
 * elin-w160 evaluation board config file based on the AM335x EVM config file header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2014, u-blox AG - http://www.u-blox.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 *
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * We have three pin mux functions that must exist.  We must be able to enable
 * uart0, for initial output and i2c0 to read the main EEPROM.  We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_i2c0_pin_mux(void);
void enable_board_pin_mux(void);
#endif
