/*
 * $Header: /home/cvspsrv/cvsroot/games/xarchon/src/audio.c,v 1.3 2001/04/19 02:16:48 mikec Exp $
 * Dan Hursh
 * $Date: 2001/04/19 02:16:48 $
 *
 * These are the audio hooks for xarchon
 *
 * Revision History:
 * $Log: audio.c,v $
 * Revision 1.3  2001/04/19 02:16:48  mikec
 * patch contributed by Mike Ciul, August 2000
 * Changes include altering rock & power point icons, improvements to field AI.
 *
 * Revision 1.1.1.1  2000/08/14 11:32:58  ahvezda
 * Initial import of xarchon
 *
 * Revision 1.5  1999/10/08 19:23:09  hursh
 * - fixed a bug that caused a segfault when init_sample_driver() failed.
 *   The array side_sample was not getting initialized.
 *
 * Revision 1.4  1999/09/29 03:03:25  hursh
 * - added a terminate function
 *
 * Revision 1.3  1999/09/29 02:27:59  hursh
 * Added code so sound can be enabled and disabled.
 * centeralized some common operations into static functions
 *
 * Revision 1.2  1999/09/26 08:59:10  hursh
 * It will now loop <theme>/generic/song1.wav for the whole game if the
 * file is available, but it only stops if someone really wins.  It plays
 * on if you use the stop option on the menu.  Need to talk to Ronen
 * about a good place to catch this.
 *
 * Revision 1.1  1999/09/26 05:31:29  hursh
 * Initial revision
 *
 */

static const char 
rscid[] = "$Id: audio.c,v 1.3 2001/04/19 02:16:48 mikec Exp $";

#include <stdlib.h>                     /* I need NULL & atexit() */
#include <string.h>                      /* strings make fun toys */
#include "audio.h"
#include "sample-driver.h"

/******************************************************************/
/* data types                                                     */
/******************************************************************/
typedef struct{
  SAMPLE sid;
  char *file;
  PLAYER pid; /* for looping */
  char looping;
} asample;

static char * path = "";
static char enabled = 1;

enum
{
  /* players */
  LIGHT = 0,
  DARK,
  
  /* generic sound events */
  START_GAME = 0,
  START_BATTLE,
  SONG,
  /*CAST_SPELL,*/
  GENERIC_END,
  
  /* side based event */
  START_TURN = 0,
  WIN_BATTLE,
  WIN_GAME,
  RELOAD,
  DAMAGE,
  SIDE_END,

  /* actor types */
  PIECE = 0, /* aka monster */
  WEAPON,

  /* piece sounds */
  PIECE_MOVE = 0,
  PIECE_NAME,  /* this isn't used yet, will be used at beginning of battle */
  PIECE_END,

  /* weapon sounds */
  WEAPON_ATTACK = 0,
  WEAPON_READY, /* this isn't used yet, will replace reload sound */ 
  WEAPON_END
};

static asample generic_sample[] = { /* generic samples */
  { SampleError, "generic/start_game.wav",   SampleError, 0 },
  { SampleError, "generic/start_battle.wav", SampleError, 0 },
  { SampleError, "generic/song1.wav",        SampleError, 0 },
  /*{ SampleError, "generic/cast_spell.wav", SampleError, 0 }*/
};

static asample light_sample[] =  { /* Light Side */
  { SampleError, "side/light/start_turn.wav", SampleError, 0 },
  { SampleError, "side/light/win_battle.wav", SampleError, 0 },
  { SampleError, "side/light/win_game.wav",   SampleError, 0 },
  { SampleError, "side/light/reload.wav",     SampleError, 0 },
  { SampleError, "side/light/damage.wav",     SampleError, 0 },
};

static asample dark_sample[] = { /* Dark Side */
  { SampleError, "side/dark/start_turn.wav", SampleError, 0 },
  { SampleError, "side/dark/win_battle.wav", SampleError, 0 },
  { SampleError, "side/dark/win_game.wav",   SampleError, 0 },
  { SampleError, "side/dark/reload.wav",     SampleError, 0 },
  { SampleError, "side/dark/damage.wav",     SampleError, 0 },
};
static asample *side_sample[] = { light_sample, dark_sample };

/* static asample piece_sample[] = {}; */
/* static asample weapon_sample[] = {}; */

/******************************************************************/
/* utilities                                                      */
/******************************************************************/
static void
play(asample* s){
  if(!audio_isenabled())
    return;
  if(s->sid != SampleError)
    start_sample(s->sid);
  return;
}

static void
loop(asample* s){
  if(s->looping)
    return;
  s->looping = 1;
  if(!audio_isenabled())
    return;
  if(s->sid != SampleError)
    s->pid = start_loop(s->sid);
  return;
}

static void
stop(asample* s){
  if(!(s->looping))
    return;
  s->looping = 0;
  if(s->pid == SampleError)
    return;
  stop_loop(s->pid);
  s->pid = SampleError;
  return;
}

static int
load_sample_set(asample* set, int count){
  int i, file_l, total_l, path_l;
  char *file;
  int status = 1;  /* we are happy */

  /* build our file name */
  file_l = 0;
  file = NULL;
  path_l = strlen(path);
  for(i = 0; i < count; i++){
    total_l = path_l + strlen(set[i].file);
    /* do we need to allocate a larger string */
    if(total_l > file_l){
      char * temp;
      temp = realloc(file, total_l + 1);
      if(temp){
        file = temp;
      }else{
        free(file);
        return(0);
      }
    strcpy(file, path);
    }
    strcpy(file + path_l, set[i].file);

    /* stop the current sound */
    if(set[i].looping && (set[i].pid != SampleError)){
      stop_loop(set[i].pid);
      set[i].pid = SampleError;
    }

    /* load the new sound */
    set[i].sid = load_sample(file);

    /* start it looping if needed */
    if(set[i].sid == -SampleError){ /* are we still happy */
      status = 0;  /* no */
    }else if(set[i].looping && audio_isenabled()){
      set[i].pid = start_loop(set[i].sid);
    }
  }
  free(file);

  /* state our mood */
  return(status);
}

static void
unload_sample_set(asample* set, int count){
  int i;

  for(i = 0; i < count; i++){
    if(set[i].looping && (set[i].pid != SampleError)){
      stop_loop(set[i].pid);
      set[i].pid = SampleError;
    }
    if(set[i].sid != SampleError){
      unload_sample(set[i].sid);
      set[i].sid = -1;
    }
  }
}

/******************************************************************/
/* the interface                                                  */
/******************************************************************/
/*
 * Initialize sound driver
 *  returns:  true if successful 
 */
int
audio_init(){
  /* open the sample device */
  if(!init_sample_driver("xarchon", NULL)){
    /* it failed */
    return(0);
  }

  /* insure we clean up */
  atexit(audio_finalize);

  /* pick an initial state */
  enabled = 1;

  /* we made it */
  return(1);
}

/*
 * Load a sound theme (this is required of audio)
 * requires:  path to the audio theme directory
 *            if the path is NULL, the current theme is reloaded
 * return true (!0) if successful
 */
int
audio_load_theme(char* theme_dir){
  int status = 1; /* I feel happy */

  /* setup theme stuff */
  if(theme_dir){
    int len1 = strlen(theme_dir);
    int len2 = len1;
    if(theme_dir[len1-1] != '/') len2++;
    if((path = malloc(len2 + 1))){
      strcpy(path, theme_dir);
      if(len2 > len1) strcat(path, "/");
    }else{
      return(0);
    }
  }

  /* load all the samples */
  load_sample_set(generic_sample, GENERIC_END);    /* generics */
  load_sample_set(side_sample[LIGHT], SIDE_END); /* light side */
  load_sample_set(side_sample[DARK], SIDE_END);   /* dark side */
  /* load_sample_set(, PIECE_END); */               /* monster */
  /* load_sample_set(, WEAPON_END); */               /* weapon */

  return(status);
}

/*
 * Unload the current theme
 */
void
audio_unload_theme(){
  /* TODO: stop all playing loops */

  /* unload the samples */
  unload_sample_set(generic_sample, GENERIC_END);
  unload_sample_set(side_sample[LIGHT], SIDE_END);
  unload_sample_set(side_sample[DARK], SIDE_END); 
  /* unload_sample_set(, PIECE_END); */               /* monster */
  /* unload_sample_set(, WEAPON_END); */               /* weapon */
}

/*
 * return (!0) in sound is enabled
 */
int
audio_isenabled(){
  return enabled;
}

/*
 * enable sound
 */
void
audio_enable(){
  if(enabled)
    return;
  enabled = 1;
  audio_load_theme(NULL);
}

/*
 * disable sound
 */
void
audio_disable(){
  if(!enabled)
    return;
  enabled = 0;
  audio_load_theme(NULL);
}

/*
 * stop all playing sounds (for quiting game)
 */
void
audio_terminate(){
  int i;
  asample* s;

  for(i = 0; i < GENERIC_END; i++){ /* generics */
    s = &(generic_sample[i]);
    if(s->looping != SampleError)
      stop(s);
  }
  for(i = 0; i < SIDE_END; i++){    /* light side */
    s = &(side_sample[LIGHT][i]);
    if(s->looping != SampleError)
      stop(s);
  }
  for(i = 0; i < SIDE_END; i++){    /* dark side */
    s = &(side_sample[DARK][i]);
    if(s->looping != SampleError)
      stop(s);
  }
  /* load_sample_set(, PIECE_END); */               /* monster */
  /* load_sample_set(, WEAPON_END); */               /* weapon */

}

/*
 * Trigger audio event for reload in battle
 * requires:  side id for side that's reloading
 */
void 
audio_player_reload(int side){
  play(&(side_sample[side][RELOAD]));
}

/*
 * Trigger audio event for beginning of a turn (actually triggered at the end)
 * requires:  side id for side that moves next
 */
void
audio_start_turn(int side){
  play(&(side_sample[side][START_TURN]));
}

/*
 * Trigger audio event for character damage during battle
 * requires:  side id of the character wounded
 */
void
audio_damage(int side){
  play(&(side_sample[side][DAMAGE]));
}

/*
 * Trigger audio event for the beginning of battle (also load cache)
 * requires: ids of pieces in the battle
 */
void
audio_start_battle(ACTOR* light, ACTOR* dark){ 
  /* I currently don't use the actors.  I would like */
  /* announce the combatants eventually. */
  play(&(generic_sample[START_BATTLE]));
}

/* 
 * Trigger audio event for the end of battle (also dump cache)
 * requires:  side id of the character who won
 */
void
audio_end_battle(int side){
  if (side != -1)
    play(&(side_sample[side][WIN_BATTLE]));
  /* TODO else it's a tie - should have a sample for that too */
}

/*
 * Trigger audio event for the start of the game
 */
void
audio_start_game(){
  play(&(generic_sample[START_GAME]));
  loop(&(generic_sample[SONG]));
}

/*
 * Trigger sound event for the end of the game
 * requires:  side id of the winning side
 */
void
audio_end_game(int side){
  if (side != -1)
    play(&(side_sample[side][WIN_GAME]));
  /* TODO else it's a tie - should have a sample for that too */
  stop(&(generic_sample[SONG]));
}

/*
 * Trigger audio event for when a character attacks
 * requires: id of the weapon being used
 */
void
audio_use_weapon(ACTOR* weapon){
  return;
}

/*
 * Signal the end of an audio event for a weapon (certain weapons only)
 * requires: id of weapon that is being stopped
 */
void
audio_stop_weapon(ACTOR* weapon){
  return;
}

/*
 * Signal that a character is moving
 * requires: id of the character moving
 */
void
audio_move_character(ACTOR* character){
  return;
}

/*
 * Signal that a character has stopped moving
 * requires: id of the character who has stopped.
 */
void
audio_stop_character(ACTOR* character){
  return;
}

/*
 * Finalization.  (what more can you say?)
 */
void
audio_finalize(){
  audio_unload_theme();
  finalize();
}

/* End $Source: /home/cvspsrv/cvsroot/games/xarchon/src/audio.c,v $ */
