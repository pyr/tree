# $Id: Makefile,v 1.4 2006/06/26 12:37:54 panda Exp $

BINDIR = /usr/local/bin
MANDIR = /usr/local/man/man
PROG   = tree
CFLAGS+= -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes

.include <bsd.prog.mk>
