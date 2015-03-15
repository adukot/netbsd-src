/*	$NetBSD$	*/
/*
 * Copyright (c) 2014 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Hiroshi Tokuda.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	_WMTREG_H_
#define	_WMTREG_H_

#define	WMT_PERIPHERALS_BASE	0xD8000000
#define	WMT_PERIPHERALS_SIZE	0x01000000	/* 16MBytes */

#define	WMT_DMA0_BASE	(WMT_PERIPHERALS_BASE + 0x00001000) /* DMA */
#define	WMT_SFM0_BASE	(WMT_PERIPHERALS_BASE + 0x00002000) /* Serial Flash Memory Controller */
#define	WMT_SPI0_BASE	(WMT_PERIPHERALS_BASE + 0x00003000) /* SPI/LPC Memory Controller */
#define	WMT_ENET0_BASE	(WMT_PERIPHERALS_BASE + 0x00004000) /* Ethernet MAC0 */
#define	WMT_ENET1_BASE	(WMT_PERIPHERALS_BASE + 0x00005000) /* Ethernet MAC1 */
#define	WMT_USB_BASE 	(WMT_PERIPHERALS_BASE + 0x00007900) /* USB Host Controller */
#define	WMT_PATA_BASE	(WMT_PERIPHERALS_BASE + 0x00008000) /* PATA Controller */
#define	WMT_NAND_BASE	(WMT_PERIPHERALS_BASE + 0x00009000) /* NAND Flash Controller */
#define	WMT_SDC_BASE	(WMT_PERIPHERALS_BASE + 0x0000A000) /* SD/SDIO/MMC Controller */
#define	WMT_MSC_BASE	(WMT_PERIPHERALS_BASE + 0x0000B000) /* Memory Stick Controller */
#define	WMT_CFC_BASE	(WMT_PERIPHERALS_BASE + 0x0000C000) /* Compact Flash Controller */
#define	WMT_SATA_BASE	(WMT_PERIPHERALS_BASE + 0x0000D000) /* SATA Controller */
#define	WMT_RTC_BASE	(WMT_PERIPHERALS_BASE + 0x00100000) /* Real Time Clock */
#define	WMT_GPIO_BASE	(WMT_PERIPHERALS_BASE + 0x00110000) /* GPIO */
#define	WMT_SCC_BASE	(WMT_PERIPHERALS_BASE + 0x00120000) /* System Configuration Control */
#define	WMT_PMC_BASE	(WMT_PERIPHERALS_BASE + 0x00130000) /* Power Management Controller */
#define	WMT_IC0_BASE	(WMT_PERIPHERALS_BASE + 0x00140000) /* Interrupt Controller 0 */
#define	WMT_IC1_BASE	(WMT_PERIPHERALS_BASE + 0x00150000) /* Interrupt Controller 1 */
#define	WMT_UART0_BASE	(WMT_PERIPHERALS_BASE + 0x00200000) /* UART0 */
#define	WMT_UART1_BASE	(WMT_PERIPHERALS_BASE + 0x00210000) /* UART1 */
#define	WMT_SPI_BASE	(WMT_PERIPHERALS_BASE + 0x00240000) /* SPI */
#define	WMT_I2C_BASE	(WMT_PERIPHERALS_BASE + 0x00280000) /* I2C */

#define	WMT_NIRQ	64

#define WMT_IOPHYSTOVIRT(a) \
    (a + 0x26000000)       /* PBASE: 0xD8000000, VBASE: 0xFE000000 */
#define	WMT_UART0_VBASE	WMT_IOPHYSTOVIRT(WMT_UART0_BASE)
#define	WMT_PERIPHERALS_VBASE \
	WMT_IOPHYSTOVIRT(WMT_PERIPHERALS_BASE)

#endif /* _WMTREG_H_ */
