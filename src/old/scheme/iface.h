/*--------------------------------------------------------------------------*/
/* interface                                                                */
/*--------------------------------------------------------------------------*/

#ifndef __MY_IFACE_H
#define __MY_IFACE_H

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define IFACE_HUMAN "human"
#define IFACE_NETWORK "network"
#define IFACE_STATIC "static"
#define IFACE_DYNAMIC "dynamic"

#define IFACE_BOARD 1
#define IFACE_FIELD 2

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
      int winner;                       /* -1 (none), 0 (light), 1 (dark) */
   } f;
} COMMAND;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void iface_start(char *light, char *dark);
void iface_turn(int which, int mode);
void iface_frame(void);
int iface_key_down(int key);
void iface_notify(COMMAND *cmd);

#endif
