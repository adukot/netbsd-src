/*	$NetBSD$	*/

/*-
 * Copyright (c) 2012 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Nick Hudson
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#define _INTR_PRIVATE

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/device.h>

#include <machine/intr.h>
#include <sys/bus.h>

#include <arm/cpufunc.h>
#include <arm/pic/picvar.h>

#include <arm/wmt/wmtreg.h>
#include <arm/wmt/obiovar.h>

static void wmt_pic_unblock_irqs(struct pic_softc *, size_t, uint32_t);
static void wmt_pic_block_irqs(struct pic_softc *, size_t, uint32_t);
static int wmt_pic_find_pending_irqs(struct pic_softc *);
static void wmt_pic_establish_irq(struct pic_softc *, struct intrsource *);
static void wmt_pic_source_name(struct pic_softc *, int, char *,
    size_t);

static int  wmt_icu_match(device_t, cfdata_t, void *);
static void wmt_icu_attach(device_t, device_t, void *);

static struct pic_ops wmt_picops = {
	.pic_unblock_irqs = wmt_pic_unblock_irqs,
	.pic_block_irqs = wmt_pic_block_irqs,
	.pic_find_pending_irqs = wmt_pic_find_pending_irqs,
	.pic_establish_irq = wmt_pic_establish_irq,
	.pic_source_name = wmt_pic_source_name,
};

struct pic_softc wmt_pic = {
	.pic_ops = &wmt_picops,
	.pic_maxsources = WMT_NIRQ,
	.pic_name = "wmt pic",
};

struct wmticu_softc {
	device_t		sc_dev;
	bus_space_tag_t		sc_iot;
	bus_space_handle_t	sc_ioh;
	struct pic_softc	*sc_pic;
};

struct wmticu_softc *wmicu_sc;

/* Interrupt Controller Destination Control Register */
#define ICDC	0x40

/* Interrupt Controller Interrupt Status Register */
#define ICIS_L	0x80  /* INT31 to INT0 */
#define ICIS_H	0x84  /* INT63 to INT32 */

#define read_icdc0(o)	\
	bus_space_read_4(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICDC + (o))
#define write_icdc0(o, v)	\
	bus_space_write_4(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICDC + (o), (v))

static const char * const wmt_sources[WMT_NIRQ] = {
	"(unused  0)",	"sdc1",		"sdc1_dma",	"(unused  3)", /* 0-3 */
	"pmc_axi_pwr",	"gpio",		"(unused  6)",	"(unused  7)", /* 4-7 */
	"kpad",		"vdma",		"eth0",		"sdc2",        /* 8-11 */
	"sdc2_dma",	"(unused 13)",	"(unused 14)",	"(unused 15)", /* 12-15 */
	"apbb",		"dma0_ch",	"i2c1",		"i2c0",	       /* 16-19 */
	"sdc0",		"sdc0_dma",	"pmc_wakeup",	"pcm",         /* 20-23 */
	"spi0",		"spi1",	        "uhdc",		"dma_ch_1",    /* 24-27 */
	"nfc",		"nfc_dma",	"uart4",	"msc_dma",     /* 28-31 */
	"uart0",	"uart1",	"dma_ch_2",	"(unused 35)", /* 32-35 */
	"ost0",		"ost1",		"ost2",		"ost3",        /* 36-39 */
	"dma_ch_3",	"dma_ch_4",	"ac97",		"uart5",       /* 40-43 */
	"ngc",		"dma_ch_5",	"dma_ch_6",	"uart2",       /* 44-47 */
	"rtc1",		"rtc2",		"uart3",	"dma_ch_7",    /* 48-51 */
	"(unused 52)",	"(unused 53)",	"(unused 54)",	"cir",         /* 52-55 */
	"ic1_irq0",	"ic1_irq1",	"ic1_irq2",	"ic1_irq3",    /* 56-59 */
	"ic1_irq4",	"ic1_irq5",	"ic1_irq6",	"ic1_irq7"     /* 60-63 */
	/* FIXME: irq 64 - 128 omitted */
};


#define	INTC_ENABLE		__BIT(3)

CFATTACH_DECL_NEW(wmicu, sizeof(struct wmticu_softc),
    wmt_icu_match, wmt_icu_attach, NULL, NULL);

static int
wmt_icu_match(device_t parent, cfdata_t cf, void *aux)
{
	return 100;
}

static void print_icdc(void);
static void
print_icdc(void) 
#if 0
{
	int i;
	uint8_t reg;
	for (i = 0; i < 64; i++) {
		printf("%d", i%10);
	}
	printf("\n");
	for (i = 0; i < 64; i++) {
		reg = bus_space_read_1(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICDC + i);
		if (reg == 0)
			printf(" ");
		else 
			printf("%d", bus_space_read_1(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICDC + i));
	}
	printf("\n");
}
#else
{
	return;
}
#endif

static void
wmt_icu_attach(device_t parent, device_t self, void *aux)
{
	struct wmticu_softc *sc = device_private(self);
	struct obio_attach_args *obio = aux;


	sc->sc_dev = self;
	sc->sc_iot = obio->obio_iot; 
	sc->sc_pic = &wmt_pic;

	if (bus_space_map(obio->obio_iot, obio->obio_addr, obio->obio_size, 0,
	    &sc->sc_ioh)) {
		aprint_error_dev(self, "unable to map device\n");
		return;
	}

	wmicu_sc = sc;
	pic_add(sc->sc_pic, 0);
	aprint_normal("\n");
#if 0
	aprint_normal("wmt_intr: wmt_icu_attach: \n");
#endif
	print_icdc();
	enable_interrupts(I32_bit);
}

void
wmt_irq_handler(void *frame)
{
	struct cpu_info * const ci = curcpu();
	const int oldipl = ci->ci_cpl;
	const uint32_t oldipl_mask = __BIT(oldipl);
	int ipl_mask = 0;
	ci->ci_data.cpu_nintr++;

	ipl_mask = wmt_pic_find_pending_irqs(&wmt_pic);

	/*
	 * Record the pending_ipls and deliver them if we can.
	 */
	if ((ipl_mask & ~oldipl_mask) > oldipl_mask)
		pic_do_pending_ints(I32_bit, oldipl, frame);
}


static void
wmt_pic_unblock_irqs(struct pic_softc *pic, size_t irqbase,
    uint32_t irq_mask)
{
	uint32_t irq_num, reg;
	int i;
#if 0	
	aprint_normal("wmt_intr: unblock: irqbase=0x%08x, irqmask=0x%08x\n", (int)irqbase, irq_mask);
#endif
	for (i = 0; i < 32; i++) {
		if (irq_mask & (1<<i)) { 
			irq_num = irqbase + i;
			reg = read_icdc0((irq_num / 4) * 4);
			reg |= INTC_ENABLE << ((irq_num % 4) * 8);
			write_icdc0((irq_num / 4) * 4, reg);
		}
        }
	print_icdc();
}

static void
wmt_pic_block_irqs(struct pic_softc *pic, size_t irqbase,
    uint32_t irq_mask)
{
	uint32_t irq_num, reg;
	int i;
#if 0 
	aprint_normal("wmt_intr: block: irqbase=0x%08x, irqmask=0x%08x\n", (int)irqbase, irq_mask);
#endif
	for (i = 0; i < 32; i++) {
		if (irq_mask & (1<<i)) { 
			irq_num = irqbase + i;
			reg = read_icdc0((irq_num / 4) * 4);
			reg &= ~(INTC_ENABLE) << ((irq_num % 4) * 8);
			write_icdc0((irq_num / 4) * 4, reg);
		}
        }
	print_icdc();
}

/*
 * Called with interrupts disabled
 */
static int
wmt_pic_find_pending_irqs(struct pic_softc *pic)
{
	int ipl = 0;
	uint32_t pending_l = bus_space_read_4(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICIS_L);
	uint32_t pending_h = bus_space_read_4(wmicu_sc->sc_iot, wmicu_sc->sc_ioh, ICIS_H);

	ipl |= pic_mark_pending_sources(pic, 0, pending_l);	
	ipl |= pic_mark_pending_sources(pic, 32, pending_h);	
#if 0
	printf("wmt_pic_find_pending_irqs: hi:%08x lo:%08x\n", pending_h, pending_l);
#endif

	return ipl;
}

static void
wmt_pic_establish_irq(struct pic_softc *pic, struct intrsource *is)
{
	/* Nothing really*/
	KASSERT(is->is_irq < WMT_NIRQ);
	KASSERT(is->is_type == IST_LEVEL);
#if 0
	aprint_normal("wmt_intr: wmt_pic_establish_irq: is->is_irq=0x%02x\n", is->is_irq);
#endif
}

static void
wmt_pic_source_name(struct pic_softc *pic, int irq, char *buf, size_t len)
{
	strlcpy(buf, wmt_sources[irq], len);
}
