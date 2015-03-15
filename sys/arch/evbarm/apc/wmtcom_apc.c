/*	$NetBSD$ */
/*
 * Copyright (c) 2002
 *	Ichiro FUKUHARA <ichiro@ichiro.org>.
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY ICHIRO FUKUHARA ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ICHIRO FUKUHARA OR THE VOICES IN HIS HEAD BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

/* Front-end of wmtcom */

#include <sys/types.h>
#include <sys/device.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/malloc.h>

#include <sys/termios.h>

#include <machine/intr.h>
#include <sys/bus.h>

#include <arm/wmt/wmt_comvar.h>
#include <arm/wmt/wmtreg.h>
#include <arm/wmt/obiovar.h>

#include <evbarm/apc/wmtcom_apcvar.h>

static int	wmtcom_apc_match(device_t, cfdata_t, void *);
static void	wmtcom_apc_attach(device_t, device_t, void *);

CFATTACH_DECL_NEW(wmtcom_apc, sizeof(struct wmtcom_softc),
    wmtcom_apc_match, wmtcom_apc_attach, NULL, NULL);

static int
wmtcom_apc_match(device_t parent, cfdata_t match, void *aux)
{
	if (strcmp(match->cf_name, "wmtcom") == 0)
		return 1;
	return 0;
}

static void
wmtcom_apc_attach(device_t parent, device_t self, void *aux)
{
	struct wmtcom_apc_softc *isc = device_private(self);
	struct wmtcom_softc *sc = &isc->sc_wmtcom;
	struct obio_attach_args *obio = aux;

	sc->sc_dev = self;
	isc->sc_iot = obio->obio_iot;
	sc->sc_iot = obio->obio_iot;
	sc->sc_baseaddr = obio->obio_addr;

	if (bus_space_map(obio->obio_iot, obio->obio_addr, obio->obio_size, 0,
			 &sc->sc_ioh)) {
		aprint_error(": unable to map device\n");
		return;
	}

	aprint_normal(": WMT UART\n");

	wmtcom_attach_subr(sc);

#ifdef POLLING_COM
	{ void* d; d = d = wmtcomintr; }
#else
	wmt_intr_establish(obio->obio_intr, IPL_SERIAL, wmtcomintr, sc);
#endif
}
