/* $NetBSD$ */
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

#define WMTCOM_TDR	0x0000 /* Transmit Data Register */ 
#define WMTCOM_RDR	0x0004 /* Receive Data Register */
#define WMTCOM_DIV	0x0008 /* Baud Rate Register */
#define WMTCOM_LCR	0x000c /* Line Control Register */
#define WMTCOM_ICR	0x0010 /* IrDA Control Register */
#define WMTCOM_IER	0x0014 /* Interrupt Enable Register */
#define WMTCOM_ISR	0x0018 /* Interrupt Status Register */
#define WMTCOM_USR	0x001c /* UART Status Register */
#define WMTCOM_FCR	0x0020 /* FIFO Control Register */
#define WMTCOM_FIDX	0x0024 /* FIFO Index Register */
#define WMTCOM_TOD	0x0028 /* UART Clock Dividor Register */
#define WMTCOM_BKR	0x002c /* Break Count-value Register */

#define LCR_PSLVERR	(1<<10)
#define LCR_BKINIT	(1<<9)
#define LCR_DMAEN	(1<<8)
#define LCR_RTS		(1<<6)
#define LCR_PTYMODE	(1<<5)
#define LCR_PTYEN	(1<<4)
#define LCR_STBLEN	(1<<3)
#define LCR_DLEN	(1<<2)
#define LCR_RXEN	(1<<1)
#define LCR_TXEN	(1<<0)

#define USR_TXFE	(1<<7)
#define USR_TXDE	(1<<6)
#define USR_BRDST	(1<<5)
#define USR_CTS		(1<<4)
#define USR_RXDRDY	(1<<3)
#define USR_RXON	(1<<2)
#define USR_TXDBSY	(1<<1)
#define USR_TXON	(1<<0)

#define IER_EBK		(1<<12)
#define IER_ERXTOUT	(1<<11)
#define IER_EMODM	(1<<10)
#define IER_EFER	(1<<9)
#define IER_EPER	(1<<8)
#define IER_ERXDOVR	(1<<7)
#define IER_ETXDUDR	(1<<6)
#define IER_ERXFF	(1<<5)
#define IER_ERXFAF	(1<<4)
#define IER_ETXFE	(1<<3)
#define IER_ETXFAE	(1<<2)
#define IER_ERXDF	(1<<1)
#define IER_ETXDE	(1<<0)

#define ISR_FER		(1<<9)
#define ISR_PER		(1<<8)
#define ISR_RXDOVR	(1<<7)
#define ISR_TXDUDR	(1<<6)
#define ISR_RXFF	(1<<5)
#define ISR_RXFAF	(1<<4)
#define ISR_TXFE	(1<<3)
#define ISR_TXFAE	(1<<2)
#define ISR_RXDF	(1<<1)
#define ISR_TXDE	(1<<0)
