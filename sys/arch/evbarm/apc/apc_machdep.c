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

#include "opt_evbarm_boardtype.h"

#include <sys/param.h>
#include <sys/device.h>
#include <sys/termios.h>
#include <sys/reboot.h>
#include <sys/sysctl.h>
#include <sys/bus.h>

#include <prop/proplib.h>

#include <dev/cons.h>

#include <uvm/uvm_extern.h>

#include <arm/arm32/machdep.h>

#include <machine/autoconf.h>
#include <machine/vmparam.h>
#include <machine/bootconfig.h>
#include <machine/pmap.h>

#include <arm/wmt/wmtreg.h>
#include <arm/wmt/wmt_comreg.h>
#include <arm/wmt/wmt_comvar.h>
#include <arm/wmt/obiovar.h>

#include <evbarm/apc/apc.h>

#include "ksyms.h"

extern int KERNEL_BASE_phys[];
extern int KERNEL_BASE_virt[];


BootConfig bootconfig;		/* Boot config storage */
static char bootargs[MAX_BOOT_STRING];
char *boot_args = NULL;

static void apc_device_register(device_t, void *);

/*
 * Macros to translate between physical and virtual for a subset of the
 * kernel address space.  *Not* for general use.
 */

#define	KERN_VTOPDIFF	((vaddr_t)KERNEL_BASE_phys - (vaddr_t)KERNEL_BASE_virt)
#define KERN_VTOPHYS(va) ((paddr_t)((vaddr_t)va + KERN_VTOPDIFF))
#define KERN_PHYSTOV(pa) ((vaddr_t)((paddr_t)pa - KERN_VTOPDIFF))

#include "opt_kgdb.h"

static void
apc_system_reset(void)
{
	cpu_reset_address = 0;
	cpu_reset_address_paddr = 0xffff0000;
	cpu_reset();
	/* NOT REACHED */
}

/*
 * Static device mappings. These peripheral registers are mapped at
 * fixed virtual addresses very early in initarm() so that we can use
 * them while booting the kernel, and stay at the same address
 * throughout whole kernel's life time.
 *
 * We use this table twice; once with bootstrap page table, and once
 * with kernel's page table which we build up in initarm().
 *
 * Since we map these registers into the bootstrap page table using
 * pmap_devmap_bootstrap() which calls pmap_map_chunk(), we map
 * registers segment-aligned and segment-rounded in order to avoid
 * using the 2nd page tables.
 */
#define _A(a)	((a) & ~L1_S_OFFSET)
#define _S(s)	(((s) + L1_S_SIZE - 1) & ~(L1_S_SIZE-1))

static const struct pmap_devmap apc_devmap[] = {
	{
		_A(APC_KERNEL_IO_VBASE),	/* 0xf2000000 */
		_A(APC_KERNEL_IO_PBASE),	/* 0xd8000000 */
		_S(APC_KERNEL_IO_VSIZE),	/* 16Mb */
		VM_PROT_READ|VM_PROT_WRITE,
		PTE_NOCACHE,
	},
	{ 0, 0, 0, 0, 0 }
};

#undef  _A
#undef  _S


void apc_putchar(char c);
void
apc_putchar(char c)
{
	int timo = 900000;

	while (!((*(volatile int *) 0xfe20001c) & 0x3))
		if (--timo == 0)
			break;

	*(volatile int *)0xfe200000  = c;
}

#ifndef MEMSIZE
#define MEMSIZE 512
#endif

#define CONSPEED B115200
#define CONMODE ((TTYDEF_CFLAG & ~(CSIZE | CSTOPB | PARENB)) | CS8) /* 8N1 */

/*
 * u_int initarm(...)
 *
 * Initial entry point on startup. This gets called before main() is
 * entered.
 * It should be responsible for setting up everything that must be
 * in place when main is called.
 * This includes
 *   Taking a copy of the boot configuration structure.
 *   Initialising the physical console so characters can be printed.
 *   Setting up page tables for the kernel
 */
u_int
initarm(void *arg)
{

	/*
	 * Heads up ... Setup the CPU / MMU / TLB functions
	 */
	if (set_cpufuncs())
		panic("cpu not recognized!");

	/* map some peripheral registers */
	pmap_devmap_bootstrap((vaddr_t) armreg_ttbr_read() & -L1_TABLE_SIZE, 
	    apc_devmap);

	cpu_domains((DOMAIN_CLIENT << (PMAP_DOMAIN_KERNEL*2)) | DOMAIN_CLIENT);

	consinit();

	/* Talk to the user */
	printf("\nNetBSD/evbarm (APC) booting ...\n");

        bootargs[0] = '\0';

#ifdef VERBOSE_INIT_ARM
	printf("initarm: Configuring system ...\n");
#endif

	bootconfig.dramblocks = 1;
	bootconfig.dram[0].address = 0x0;
	bootconfig.dram[0].pages = (MEMSIZE * 1024 * 1024) / PAGE_SIZE;

	arm32_bootmem_init(bootconfig.dram[0].address,
	    bootconfig.dram[0].pages * PAGE_SIZE, (uintptr_t)KERNEL_BASE_phys);

	arm32_kernel_vm_init(KERNEL_VM_BASE, ARM_VECTORS_HIGH, 0, apc_devmap,
	    false);

	cpu_reset_address = apc_system_reset;

#ifdef VERBOSE_INIT_ARM
	printf("done.\n");
#endif

#ifdef __HAVE_MEMORY_DISK__
	md_root_setconf(memory_disk, sizeof memory_disk);
#endif

#ifdef BOOTHOWTO
	boothowto |= BOOTHOWTO;
#endif

	/* we've a specific device_register routine */
	evbarm_device_register = apc_device_register;

	return initarm_common(KERNEL_VM_BASE, KERNEL_VM_SIZE, NULL, 0);
}



void
consinit(void)
{
	static int consinit_called;

	if (consinit_called)
		return;
	consinit_called = 1;

	pmap_devmap_register(apc_devmap);

	if (wmtcomcnattach(&obio_bs_tag, WMT_UART0_BASE, WMT_UART0_VBASE, CONSPEED, CONMODE)) {
		panic("can't init serial console %lx", (long) WMT_UART0_BASE);
        }
}

static void
apc_device_register(device_t dev, void *aux)
{
	/* none */
}
