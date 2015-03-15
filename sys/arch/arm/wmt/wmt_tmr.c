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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/kernel.h>
#include <sys/timetc.h>
#include <sys/bus.h>

#include <arm/wmt/wmt_intr.h>
#include <arm/wmt/wmtreg.h>
#include <arm/wmt/obiovar.h>

#define	WMT_OSTMR_M0	0x00
#define	WMT_OSTMR_M1	0x04
#define	WMT_OSTMR_M2	0x08
#define	WMT_OSTMR_M3	0x0C
#define	WMT_OSTMR_CR	0x10
#define	WMT_OSTMR_TS	0x14
#define	WMT_OSTMR_TW	0x18
#define	WMT_OSTMR_TI	0x1C
#define	WMT_OSTMR_TC	0x20
#define	WMT_OSTMR_TA	0x24

#define	WMT_OSTMR_HZ	(3 * 1000 * 1000)

static const uint32_t counts_per_usec = (WMT_OSTMR_HZ / 1000000);
static uint32_t counts_per_hz = ~0;

struct wmttmr_softc {
	device_t		sc_dev;
	bus_space_tag_t sc_iot;
	bus_space_handle_t sc_ioh;
	int	sc_intr;
};
 
static int wmtmr_match(device_t, cfdata_t, void *);
static void wmtmr_attach(device_t, device_t, void *);

static int clockhandler(void *);

static u_int wmttmr_get_timecount(struct timecounter *);

static struct wmttmr_softc *wmttmr_sc;

static struct timecounter wmttmr_timecounter = {
	.tc_get_timecount = wmttmr_get_timecount,
	.tc_poll_pps = 0,
	.tc_counter_mask = ~0u,
	.tc_frequency = WMT_OSTMR_HZ,
	.tc_name = NULL,			/* set by attach */
	.tc_quality = 100,
	.tc_priv = NULL,
	.tc_next = NULL,
};

uint32_t get_current(void);

CFATTACH_DECL_NEW(wmtmr, sizeof(struct wmttmr_softc),
    wmtmr_match, wmtmr_attach, NULL, NULL); 

/* ARGSUSED */
static int
wmtmr_match(device_t parent, cfdata_t match, void *aux)
{

	return 1;
}

static void
wmtmr_attach(device_t parent, device_t self, void *aux)
{
	struct obio_attach_args *obio = aux;
        struct wmttmr_softc *sc = device_private(self);

	aprint_naive("\n");
	aprint_normal(": WMT OS Timer\n");

	if (wmttmr_sc == NULL)
		wmttmr_sc = sc;

	sc->sc_dev = self;
	sc->sc_iot = obio->obio_iot;
	sc->sc_intr = obio->obio_intr;

	if (bus_space_map(obio->obio_iot, obio->obio_addr, obio->obio_size, 0,
	    &sc->sc_ioh)) {
		aprint_error_dev(sc->sc_dev, "unable to map device\n");
		return;
	}

	wmttmr_timecounter.tc_name = device_xname(self);
}

#define OSTA_MWA1	__BIT(1)
#define OSTA_CWA	__BIT(4)
#define OSTA_RCA	__BIT(5)

#define OSTI_E1		__BIT(1)

#define OSTC_ENABLE	__BIT(0)
#define OSTC_RDREQ	__BIT(1)

#define OSTS_M1		__BIT(1)

void
cpu_initclocks(void)
{
	struct wmttmr_softc *sc = wmttmr_sc;
	void *clock_ih;

	KASSERT(sc != NULL);

	wmttmr_timecounter.tc_priv = sc;

	counts_per_hz = WMT_OSTMR_HZ / hz;
	
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC, 0); /* stop os timer */

	while(bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TA) & OSTA_MWA1)
		;
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_M1, counts_per_hz);

	/* clear status on all timers */
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TS, 0xF);
#if 0 
	printf("wmt_tmr: cpu_initclocks. sc_intr=%d, ", sc->sc_intr);
	printf("counts_per_hz = %d, ", counts_per_hz);
	printf("WMT_OSTMR_TC=0x%08x, ", bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC));
	printf("WMT_OSTMR_M1=0x%08x\n", bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_M1));
#endif
	clock_ih = wmt_intr_establish(sc->sc_intr, IPL_CLOCK,
	    clockhandler, NULL);
	if (clock_ih == NULL)
		panic("%s: unable to register timer interrupt", __func__);

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TI, OSTI_E1); /* enable intr timer1 */

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC, OSTC_ENABLE); /* enable os timer */

	/* Initaialize counter register */
	while(bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TA) & OSTA_CWA)
		;
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_CR, 0);
#if 0
	printf("wmt_tmr: WMT_OSTMR_TI = %08x, ", bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TI));
	printf("WMT_OSTMR_TC=0x%08x, ", bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC));
#endif

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC,
	  bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC) | OSTC_RDREQ);
	while (bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TA) & OSTA_RCA) {
		;
	}
#if 0
	printf("WMT_OSTMR_CR=0x%08x\n", bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_CR));
#endif
	tc_init(&wmttmr_timecounter);
#if 0
	printf("wmt_tmr: tc_init done.\n");
#endif
}

uint32_t
get_current(void)
{
	struct wmttmr_softc *sc = wmttmr_sc;

	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC,
	  bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TC) | OSTC_RDREQ);

	while (bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TA) & OSTA_RCA) {
		;
	}

	return bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_CR);
}

void
delay(unsigned int n)
{
	struct wmttmr_softc *sc = wmttmr_sc;
	uint32_t last, curr;
	uint32_t delta, usecs;

#if 0
	KASSERT(sc != NULL); 
	printf("wmttmr: delay(%d)\n", n);
#endif
	if (sc == NULL) {
		printf("wmttmr: sc == NULL\n");
		return;
	}

	last = get_current();
	delta = usecs = 0;
	while (n > usecs) {
		curr = get_current();
		/* Check to see if the timer has wrapped around. */
		if (curr < last)
			delta += curr + (UINT32_MAX - last);
		else
			delta += curr - last;

		last = curr;

		if (delta >= counts_per_usec) {
			usecs += delta / counts_per_usec;
			delta %= counts_per_usec;
		}
	}
}

/*
 * clockhandler:
 *
 *	Handle the hardclock interrupt.
 */
static int
clockhandler(void *arg)
{
	struct wmttmr_softc *sc = wmttmr_sc;
	struct clockframe *frame = arg;
	uint32_t curr;
#if 0
	printf("wmtmr: clockhandler: hardclock()\n");
#endif
	hardclock(frame);
	while(bus_space_read_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TA) & OSTA_MWA1)
		;
	curr = get_current();
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_TS, OSTS_M1);
	bus_space_write_4(sc->sc_iot, sc->sc_ioh, WMT_OSTMR_M1, curr + counts_per_hz);

	return 1;
}

void
setstatclockrate(int newhz)
{
}

static u_int
wmttmr_get_timecount(struct timecounter *tc)
{
	uint32_t curr;
	curr = get_current();
#if 0
	printf("wmtmr: wmt_get_timecount %d\n", curr);
#endif
	return curr;
}

