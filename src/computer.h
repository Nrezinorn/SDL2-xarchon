/*--------------------------------------------------------------------------*/
/* computer                                                                 */
/*--------------------------------------------------------------------------*/

#ifndef __MY_COMPUTER_H
#define __MY_COMPUTER_H

#include <stdio.h>

#include "actors.h"
#include "iface.h"

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   char rules[33];                      /* computer rules file */
   int old_board_mode;                  /* board is AI- or rule- based */
   int data_num;                        /* which data structure to use */
} COMPUTER_CONFIG;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void computer_start(int side, COMPUTER_CONFIG *config);
void computer_turn(int side, int mode, COMMAND *cmd);
void computer_config_read(FILE *fp, COMPUTER_CONFIG *config);
void computer_config_write(FILE *fp, COMPUTER_CONFIG *config);
void computer_config_edit(COMPUTER_CONFIG *_config);
int computer_field_score(ACTOR *attacker, ACTOR *defender,
                         int *attacker_damage, int *defender_damage,
                         int *attacker_hits, int *defender_hits);

#endif
