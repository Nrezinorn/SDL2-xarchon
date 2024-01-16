/*
 * $Header: /home/cvspsrv/cvsroot/games/xarchon/src/audio.h,v 1.1.1.1 2000/08/14 11:32:58 ahvezda Exp $
 * Dan Hursh
 * $Date: 2000/08/14 11:32:58 $
 *
 * These are the audio hooks for xarchon
 *
 * Revision History:
 * $Log: audio.h,v $
 * Revision 1.1.1.1  2000/08/14 11:32:58  ahvezda
 * Initial import of xarchon
 *
 * Revision 1.4  1999/09/29 02:32:51  hursh
 * - added a termination function for stopping the game
 *
 * Revision 1.3  1999/09/29 01:33:53  hursh
 * fixed a typo
 *
 * Revision 1.2  1999/09/29 00:41:37  hursh
 * added functions for enabling and disabling sound.
 *
 * Revision 1.1  1999/09/26 05:39:34  hursh
 * Initial revision
 *
 *
 */
#include "actors.h"

#ifndef __MY_AUDIO_H
#define __MY_AUDIO_H "@(#)audio.h $Revision: 1.1.1.1 $"

/*
 * Initialize sound driver
 *  returns:  true (!0) if successful 
 */
int
audio_init();

/*
 * Load a sound theme (this is required of audio)
 * requires:  path to the audio theme directory
 * return true (!0) if successful
 */
int
audio_load_theme(char* theme_dir);

/*
 * Unload the current theme
 */
void
audio_unload_theme();

/*
 * return (!0) if audio is enabled
 */
int
audio_isenabled();

/*
 * enable audio
 */
void
audio_enable();

/*
 * disable audio
 */
void
audio_disable();

/*
 * stop all playing sounds (for quiting game)
 */
void
audio_terminate();

/*
 * Trigger audio event for reload in battle
 * requires:  side id for side that's reloading
 */
void
audio_player_reload(int side);

/*
 * Trigger audio event for beginning of a turn (actually triggered at the end)
 * requires:  side id for side that moves next
 */
void
audio_start_turn(int side);

/*
 * Trigger audio event for character damage during battle
 * requires:  side id of the character wounded
 */
void
audio_damage(int side);

/*
 * Trigger audio event for the beginning of battle (also load cache)
 * requires: ids of pieces in the battle
 */
void
audio_start_battle(ACTOR* light, ACTOR* dark);

/* 
 * Trigger audio event for the end of battle (also dump cache)
 * requires:  side id of the character who won
 */
void
audio_end_battle(int side);

/*
 * Trigger audio event for the start of the game
 */
void
audio_start_game();

/*
 * Trigger sound event for the end of the game
 * requires:  side id of the winning side
 */
void
audio_end_game(int side);

/*
 * Trigger audio event for when a character attacks
 * requires: id of the weapon being used
 */
void
audio_use_weapon(ACTOR* weapon);

/*
 * Signal the end of an audio event for a weapon (think impact)
 * requires: id of weapon that is being stopped
 */
void
audio_stop_weapon(ACTOR* weapon);

/*
 * Signal that a character is moving
 * requires: id of the character moving
 */
void
audio_move_character(ACTOR* character);

/*
 * Signal that a character has stopped moving
 * requires: id of the character who has stopped.
 */
void
audio_stop_character(ACTOR* character);

/*
 * Finalization.  (what more can you say?)
 */
void
audio_finalize();

#endif /* __MY_AUDIO_H */
/* $Source: /home/cvspsrv/cvsroot/games/xarchon/src/audio.h,v $ */
