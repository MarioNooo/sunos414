#
#	@(#)master_4.x 1.1 94/10/31 SMI
#
#	Master Makefile for suninstall for SunOS 4.x
#
#	Anything defined in this Makefile should be global in nature.
#

#
#	System architecture
#
ARCH=		`arch`

#
#	Global CFLAGS
#
G_CFLAGS=	-O

#
#	Global LINTFLAGS
#
G_LINTFLAGS=

#
#	Object list for lib/Makefile
#
OBJS=		$(OBJS_COMM)

#
#	Software architecture
#
#		4.x - SunOS 4.0 and direct derivatives
#		MLS - SunOS MLS
#
SOFT_ARCH=	4.x

#
#	Source list for lib/Makefile
#
#		SRCS_COMM - sources common to all architectures
#		SRCS_MLS  - sources for SunOS MLS
#
SRCS=		$(SRCS_COMM)
