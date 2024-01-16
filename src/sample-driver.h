/*
 * $Header: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver.h,v 1.1.1.1 2000/08/14 11:32:58 ahvezda Exp $
 * Dan Hursh
 * $Date: 2000/08/14 11:32:58 $
 *
 * This is the generic sound driver interface for xarchon
 *
 * Revision History:
 * $Log: sample-driver.h,v $
 * Revision 1.1.1.1  2000/08/14 11:32:58  ahvezda
 * Initial import of xarchon
 *
 * Revision 1.1  1999/09/26 05:48:31  hursh
 * Initial revision
 *
 */

#ifndef __MY_SAMPLE_DRIVER_H
#define __MY_SAMPLE_DRIVER_H "@(#)sample-driver.h $Revision: 1.1.1.1 $"


typedef int SAMPLE;              /* type for a sample */
typedef int PLAYER;              /* type for a playing sample */
#define SampleError (-1)         /* error code */

/* driver specific initializer */
int    init_sample_driver();     /* return 0 on failure */

SAMPLE load_sample(char *name);  /* load a sample and return a unique id */
void   unload_sample(SAMPLE s);  /* unload sample */
PLAYER start_sample(SAMPLE s);   /* play sample */
PLAYER start_loop(SAMPLE s);     /* loop sample continuously */
void   stop_sample(PLAYER p);    /* stop sample immediately */
void   stop_loop(PLAYER p);      /* stop looping sample at end */
void   finalize();               /* disable sound and cleanup */


#endif /* __MY_SAMPLE_DRIVER */
/* $Source: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver.h,v $ */
