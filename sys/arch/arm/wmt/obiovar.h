/*	$NetBSD$	*/

/*
 * Copyright (c) 2002, 2003 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Jason R. Thorpe for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed for the NetBSD Project by
 *	Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _OBIOVAR_H_
#define	_OBIOVAR_H_

struct obio_attach_args {
	bus_space_tag_t obio_iot;		/* bus space tag */
	bus_addr_t      obio_addr;		/* address of device */
	bus_size_t      obio_size;		/* size of device */
	int             obio_intr;		/* irq */
	bus_dma_tag_t	obio_dmat;
	int             obio_width;		/* bus width */
};

typedef struct obio_softc {
	device_t                sc_dev;
        bus_dma_tag_t           sc_dmat;
        struct arm32_dma_range  sc_dmarange;
        bus_space_tag_t         sc_iot;
        bus_space_handle_t      sc_ioh;
        bus_addr_t              sc_base;
        bus_size_t              sc_size;
#if 0
        /* Bus space, DMA, and PCI tags for the PCI bus. */
        bus_space_handle_t      sc_pcicfg_ioh;
        struct arm32_bus_dma_tag sc_pci_dmat;
        struct arm32_pci_chipset sc_pci_chipset;
#endif
} obio_softc_t;

extern struct bus_space obio_bs_tag;
extern struct arm32_bus_dma_tag obio_bus_dma_tag;
#endif /* _OBIOVAR_H_ */
