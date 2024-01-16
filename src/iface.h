/*--------------------------------------------------------------------------*/
/* interface                                                                */
/*--------------------------------------------------------------------------*/

#ifndef __MY_IFACE_H
#define __MY_IFACE_H

#include <stdio.h>

#include "list.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

enum {
   IFACE_HUMAN = 1,
   IFACE_COMPUTER,
   IFACE_NETWORK,
   IFACE_AI_COMPUTER,
   NUM_IFACES
};

enum {
   IFACE_BOARD = 1,
   IFACE_FIELD_START,
   IFACE_FIELD,
   IFACE_FIELD_STOP
};

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef union {                         /* computer command */
   struct {
      int spell;                        /* spell, or 0 for simple move */
      int x1, y1;                       /* source cell (move, spells t/x) */
      int x2, y2;                       /* target cell (move, t/h/x/l/v/m) */
   } b;
   struct {
      int dir;                          /* direction of command */
      int fire;                         /* command modifier: fire or move */
   } f;
} COMMAND;

typedef struct {
   LIST_ELEM list_elem;
   char name[33];                       /* player's name */
   int type;                            /* which kind of player */
   void *human;                         /* -> HUMAN_CONFIG */
   void *computer;                      /* -> COMPUTER_CONFIG */
   void *network;                       /* -> NETWORK_CONFIG */
} IFACE_PLAYER;

typedef struct {
   char light_name[33], dark_name[33];  /* default ifaces for each side */
   int light_first;                     /* if light side goes first */
   LIST players;                        /* elements of IFACE_PLAYER */
} IFACE_CONFIG;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

char *iface_start(int *light_first);
void iface_turn(int side_num, int mode);
void iface_frame(void);
int iface_key_down(int key);
void iface_notify_computer(int mode);
int iface_is_pausable(void);
IFACE_CONFIG *iface_get_config(void);
IFACE_PLAYER *iface_new_player(void);
void iface_delete_player(IFACE_PLAYER *player);
void iface_config_read(FILE *fp);
void iface_config_write(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* __MY_IFACE_H */
