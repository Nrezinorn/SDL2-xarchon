/*
 * $Header: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver-esd.c,v 1.3 2001/04/19 02:16:48 mikec Exp $
 * Dan Hursh
 * $Date: 2001/04/19 02:16:48 $
 *
 * This implementation SUCKS!
 * I need to do thinks that EsounD doesn't do.
 * This will be replaced eventually but it is good enough for today.
 * Some good rules to follow here would be to only run one loop of
 * a given sample, and don't count on being able to stop a playing sample.
 *
 * Revision History:
 * $Log: sample-driver-esd.c,v $
 * Revision 1.3  2001/04/19 02:16:48  mikec
 * patch contributed by Mike Ciul, August 2000
 * Changes include altering rock & power point icons, improvements to field AI.
 *
 * Revision 1.1.1.1  2000/08/14 11:32:58  ahvezda
 * Initial import of xarchon
 *
 * Revision 1.2  1999/10/02 06:10:32  hursh
 * - life lesson -- even though a pointer may not point to a resource
 *   that needs to be deallocated, you should still set it equal to NULL
 *   when you through with it.
 *
 * Revision 1.1  1999/09/26 05:42:30  hursh
 * Initial revision
 *
 *
 */

static const char 
rscid[] = "$Id: sample-driver-esd.c,v 1.3 2001/04/19 02:16:48 mikec Exp $";

#include "sample-driver.h" /* the interface we want to implement      */
#include <esd.h>           /* the interface we want to implement over */
#include <stdio.h>         /* even good programs must gripe           */

#include <unistd.h>        /* we need to know if the file is there    */
#include <sys/stat.h>      /*  don't we?                              */
#include <string.h>        /* to play games with filenames, etc       */
#include <stdlib.h>        /* atexit lets me clean up after myself    */

/* global var (yes I know.  This is a first try.) */
static int sock = -1;
static char* _prefix = "";

/*********************************************************************/
/* book keeping                                                      */
/*********************************************************************/
/* sample table */
typedef struct{
  int esd_id;
  int looping;
} sample_record;
static sample_record *begin = NULL, *current = NULL, *end = NULL;


/* init rows in list */
static void
init_rows(sample_record *start, sample_record *stop){
  for(;start != end; start++){
    start->esd_id = SampleError;
    start->looping = 0;
  }
}

/* init list */
static int
init_sample_list(size_t size){
  if((begin = current = malloc(sizeof(sample_record) * size))){
    init_rows(begin, end = begin + size);
    return 1;
  }else{
    begin = current = end;
    return 0;
  }
}

/* search for first matching entry in the list */
static sample_record*
search_list(int id, sample_record *start, sample_record *stop){
  for(; (start != stop) && (start->esd_id != id); start++);
  return start;
}

/* add a new sample to our table */
static SAMPLE
alloc_sample(int id){
  sample_record *free;
  size_t diff;

  /* search for a free row */
  free = search_list(SampleError, current, end);
  if(free != end){
    current = free + 1;
    free->looping = 0;
    return free->esd_id = id;
  }
  free = search_list(SampleError, begin, current);
  if(free != current){
    current = free + 1;
    free->looping = 0;
    return free->esd_id = id;
  }

  /* did not find a free row, so reallocate */
  diff = end - begin;
  if((free = realloc(begin, diff * 2 + 1))){
    begin = free;
    free += diff;
    end = diff + free + 1;
    current = free + 1;
    init_rows(current, end);
    free->looping = 0;
    return(free->esd_id = id);
  }

  /* realloc failed */
  current = begin;
  return SampleError;
}
  
/* remove a sample from our list */
static void
free_sample(int id){
  current = search_list(id, begin, end);
  if(current != end){
    current->looping = 0;
    current->esd_id = SampleError;
  }
}

/* a good cleanup step */
static void
free_all_samples(){
  for(current = begin; current != end; current++)
    if(current->esd_id != SampleError){
      esd_sample_stop(sock, current->esd_id); /* esd_sample_free won't */
      esd_sample_free(sock, current->esd_id); /* stop sample from looping */
      current->looping = 0;
      current->esd_id = SampleError;
    }
}

/*********************************************************************/
/* initialize sound device                                           */
/*********************************************************************/
int
init_sample_driver(/* char* prefix, char* host */){
  char prefix[] = "xarchon";
  char *host = NULL;

  /* we must be certain to cleanup after ourselves */
  atexit(free_all_samples);  /* what do you do if it fails? */

  /* did they give us a prefix */
  if(prefix){
    /* yes, malloc a string to copy into */
    if((_prefix = malloc(strlen(prefix) + 1))){
      /* malloc worked so copy it */
      strcpy(_prefix, prefix);
    }else{
      /* malloc failed, so do we */
      return(0);
    }
  }

  /* initialize our list of allocates samples */
  if(!init_sample_list(64)){  /* TODO: this should be fine tuned */
    /* it fail, so do we */
    return(0);
  }

  /* open the socket */
  sock = esd_open_sound(host);
  if(sock <= 0){
    /* it failed, so do we */
    return(0);
  }

  /* we made it */
  return 1;
}

/*********************************************************************/
/* Interface Functions                                               */
/*********************************************************************/
/* load a sample and return a unique id */
SAMPLE 
load_sample(char *name /*, int id*/){
  struct stat buf;
  SAMPLE sample_id;
  char *prefix;
  int id = 0;

  /* stat the file */
  if(stat(name, &buf)){
    /* it's not there */
    return(SampleError);
  }

  /* is it a file */
  if(!S_ISREG(buf.st_mode)){
    /* no, we fail */
    return(SampleError);
  }

  /* create the prefix */
  id &= 0xFF;
  if((prefix = malloc(strlen(_prefix) + 5 ))){
    sprintf(prefix, "%s-%d", _prefix, id);
  }else{
    return(SampleError);
  }

  /* load the sample */
  sample_id = esd_file_cache(sock, prefix, name);
  free(prefix);

  /* did it fail */
  if ( sample_id < 0 ) {
    /* yes, so did we */
    return(SampleError);
  }

  /* record the esd sample id */
  if(alloc_sample(sample_id)){
    /* return the sample id */
    return(sample_id);
  }else{
    /* problem recording sample id, better unload it */
    esd_sample_free(sock, sample_id);
    return(SampleError);
  }
}

/* unload sample */
void   
unload_sample(SAMPLE s){
  if(s > SampleError){
    sample_record *r;
    r = search_list(s, begin, end);
    if(r->looping){
      esd_sample_stop(sock, s); /* stop sample from looping */
      r->looping = 0;
    }
    free_sample(s);
  esd_sample_free(sock, s); /* eventually esd will do the above step for us */
  }
}

/* play sample */
PLAYER
start_sample(SAMPLE s){
  esd_sample_play(sock, s);  
  return(s);
}

/* loop sample continuously */
PLAYER
start_loop(SAMPLE s){
  if(s > SampleError){
    sample_record *r;
    r = search_list(s, begin, end);
    if(r == end) return SampleError;
    if(!r->looping) esd_sample_loop(sock, s);
    r->looping++;
    if(!r->looping) r->looping--;
  }
  return(s);
}

/* stop sample immediately */
void   
stop_sample(PLAYER p){
  /* esd_sample_kill(sock, s); TODO: flat out broken */
}

/* stop looping sample at end */
void   
stop_loop(PLAYER p){
  if(p > SampleError){
    sample_record *r;
    r = search_list(p, begin, end); 
    if(r != end) 
      if(r->looping){
        r->looping--;
        if(!r->looping)
          esd_sample_stop(sock, p);
      }
  }
}

/* disable sound and cleanup */
void   
finalize(){
  if(sock > -1){
    free_all_samples();
    esd_close(sock);
    sock = -1;
  }
  if(_prefix){
    free(_prefix);
    _prefix = NULL;
  }
  if(begin){
    free(begin);
    begin = NULL;
  }
  current = end = NULL;
}

/* $Source: /home/cvspsrv/cvsroot/games/xarchon/src/sample-driver-esd.c,v $ */
