# $NetBSD: Makefile.inc,v 1.1 2014/09/19 17:36:25 matt Exp $

CPPFLAGS+=	-I${ARCHDIR}
CPPFLAGS+=	-DHAVE_INITFINI_ARRAY
#CPPFLAGS+=	-DELF_NOTE_MARCH_DESC=\"${CSU_MACHINE_ARCH}\"
