/*--------------------------------------------------------------------------*/
/* human interface support                                                  */
/*--------------------------------------------------------------------------*/

#ifndef __MY_HUMAN_H
#define __MY_HUMAN_H

#include <X11/Xlib.h>
#include <stdio.h>

#include "actors.h"
#include "iface.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define HUMAN_JOY_MAX_DEV  4

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   KeySym keys[2][STATE_MOVE_COUNT];    /* [0] for light, [1] for dark */
   int non_keyboard;                    /* -1 = keyboard only */
                                        /* >= 0 && < JOY_MAX_DEV = joystick */
                                        /* >= JOY_MAX_DEV = mouse */
} HUMAN_CONFIG;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void human_start(int side, HUMAN_CONFIG *_config);
void human_turn(int side, int mode, COMMAND *cmd);
void human_frame(int *keys_down);
void human_config_read(FILE *fp, HUMAN_CONFIG *config);
void human_config_write(FILE *fp, HUMAN_CONFIG *config);
void human_config_edit(HUMAN_CONFIG *config);
int human_joystick_init(int num);

#ifdef __cplusplus
}
#endif

#endif /* __MY_HUMAN_H */
