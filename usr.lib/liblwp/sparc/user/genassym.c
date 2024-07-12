/* Copyright (C) 1987 Sun Microsystems Inc. */
/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this source code without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * THIS PROGRAM CONTAINS SOURCE CODE COPYRIGHTED BY SUN MICROSYSTEMS, INC.
 * AND IS LICENSED TO SUNSOFT, INC., A SUBSIDIARY OF SUN MICROSYSTEMS, INC.
 * SUN MICROSYSTEMS, INC., MAKES NO REPRESENTATIONS ABOUT THE SUITABLITY
 * OF SUCH SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT
 * EXPRESS OR IMPLIED WARRANTY OF ANY KIND.  SUN MICROSYSTEMS, INC. DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO SUCH SOURCE CODE, INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN
 * NO EVENT SHALL SUN MICROSYSTEMS, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM USE OF SUCH SOURCE CODE, REGARDLESS OF THE THEORY OF LIABILITY.
 * 
 * This source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction, 
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS
 * SOURCE CODE OR ANY PART THEREOF.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California 94043
 */
#include "common.h"
#include "queue.h"
#include "asynch.h"
#include "machsig.h"
#include "machdep.h"
#include "lwperror.h"
#include "cntxt.h"
#include "message.h"
#include "process.h"
#ifndef lint
SCCSID(@(#) genassym.c 1.1 94/10/31 Copyr 1987 Sun Micro);
#endif lint

/*
 * generate REGOFFSET for low.h
 */
main()
{
	register lwp_t *l = (lwp_t *)0;

	printf("#define SP_OFFSET\t%d\n", &l->mch_sp);
	printf("#define PC_OFFSET\t%d\n", &l->mch_pc);
	printf("#define Y_OFFSET\t%d\n", &l->mch_y);
	printf("#define G2_OFFSET\t%d\n", &(l->mch_globals[1]));
	printf("#define	O0_OFFSET\t%d\n", &(l->mch_oregs[0]));
	printf("#define CKSTKOVERHEAD\t%d\n", STKOVERHEAD);
	exit(0);
}
