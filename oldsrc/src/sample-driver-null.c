/*
 * $Header: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver-null.c,v 1.1.1.1 2000/08/14 11:32:58 ahvezda Exp $
 * Dan Hursh
 * $Date: 2000/08/14 11:32:58 $
 *
 * This is the null sounds driver for systems that can't use any other
 * sound driver.
 *
 * Revision History:
 * $Log: sample-driver-null.c,v $
 * Revision 1.1.1.1  2000/08/14 11:32:58  ahvezda
 * Initial import of xarchon
 *
 * Revision 1.2  1999/09/26 05:51:51  hursh
 * When you check a file in using template RCS tags, make sure to tailor
 * all the necessary information, or you'll waste a revision correcting
 * it later.  Trust me.
 *
 * Revision 1.1  1999/09/26 05:44:50  hursh
 * Initial revision
 *
 */

static const char 
rscid[] = "$Id: sample-driver-null.c,v 1.1.1.1 2000/08/14 11:32:58 ahvezda Exp $";

#include "sample-driver.h"

/* driver specific initializer */
int    init_sample_driver(){     /* return 0 on failure */
  return(1);                     /* but we never fail   */
}

SAMPLE load_sample(char *name){  /* load a sample and return a unique id */
  return(1);                     /* ya, we're loaded                     */
}

void unload_sample(SAMPLE s){    /* unload sample */
  return;
}

PLAYER start_sample(SAMPLE s){   /* play sample */
  return(1);
}

PLAYER start_loop(SAMPLE s){     /* loop sample continuously */
  return(1);
}

void stop_sample(PLAYER p){      /* stop sample immediately */
  return;
}

void stop_loop(PLAYER p){        /* stop looping sample at end */
  return;
}

void finalize(){                 /* disable sound and cleanup */
  return;
}

/* $Source: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver-null.c,v $ */
