/*	$NetBSD$	*/

/*
 * Copyright (c) 2001, 2002, 2003 Wasabi Systems, Inc.
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <machine/cpu.h>
#include <sys/bus.h>

#include <arm/mainbus/mainbus.h>
#include <arm/wmt/wmtreg.h>
#include <arm/wmt/obiovar.h>


#include "locators.h"

int	obio_match(device_t, cfdata_t, void *);
void	obio_attach(device_t, device_t, void *);

CFATTACH_DECL_NEW(obio, sizeof(struct obio_softc),
    obio_match, obio_attach, NULL, NULL);

int	obio_print(void *, const char *);
int	obio_search(device_t, cfdata_t, const int *, void *);

/* there can be only one */
bool	obio_found;

int
obio_match(device_t parent, cfdata_t cf, void *aux)
{

	if (obio_found)
		return 0;
	return 1;
}

void
obio_attach(device_t parent, device_t self, void *aux)
{
	struct obio_softc *sc = device_private(self);
	struct mainbus_attach_args *mb = (struct mainbus_attach_args *) aux;

	sc->sc_dev = self;
	sc->sc_iot = &obio_bs_tag;
	sc->sc_ioh = 0;
	sc->sc_dmat = &obio_bus_dma_tag;
	sc->sc_base = mb->mb_iobase;
	sc->sc_size = mb->mb_iosize;

	obio_found = true;

	aprint_naive("\n");
	aprint_normal("\n");

	/*
	 * Attach all on-board devices as described in the kernel
	 * configuration file.
	 */
	config_search_ia(obio_search, self, "obio", NULL);
}

int
obio_print(void *aux, const char *pnp)
{
	struct obio_attach_args *obio = aux;

	aprint_normal(" addr 0x%08lx", obio->obio_addr);
	aprint_normal("-0x%08lx", obio->obio_addr + (obio->obio_size - 1));
	if (obio->obio_width != OBIOCF_WIDTH_DEFAULT)
		aprint_normal(" width %d", obio->obio_width);
	if (obio->obio_intr != OBIOCF_INTR_DEFAULT)
		aprint_normal(" intr %d ", obio->obio_intr);

	return UNCONF;
}

int
obio_search(device_t parent, cfdata_t cf, const int *ldesc, void *aux)
{
	struct obio_softc * const sc = device_private(parent);
	struct obio_attach_args obio;

	obio.obio_iot = &obio_bs_tag;
	obio.obio_dmat = sc->sc_dmat;
	obio.obio_addr = cf->cf_loc[OBIOCF_ADDR];
	obio.obio_size = cf->cf_loc[OBIOCF_SIZE];
	obio.obio_width = cf->cf_loc[OBIOCF_WIDTH];
	obio.obio_intr = cf->cf_loc[OBIOCF_INTR];


	if (config_match(parent, cf, &obio) > 0)
		config_attach(parent, cf, &obio, obio_print);

	return 0;
}
