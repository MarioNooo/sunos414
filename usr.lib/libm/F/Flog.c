#ifndef lint
static	char sccsid[] = "@(#)Flog.c 1.1 94/10/31 SMI";
#endif

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

# include <math.h>

#define func log

FLOATFUNCTIONTYPE F/**/func/**/(x)
FLOATPARAMETER x;
{
	return r_/**/func/**/_(&x);
}
