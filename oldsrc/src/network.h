/*--------------------------------------------------------------------------*/
/* network interface support                                                */
/*--------------------------------------------------------------------------*/

#ifndef __MY_NETWORK_H
#define __MY_NETWORK_H

#include <stdio.h>

#include "iface.h"

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   char address[65];                    /* host address */
   int port;                            /* port number */
} NETWORK_CONFIG;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

int network_start(int side, NETWORK_CONFIG *_config);
void network_turn(int side, int mode, COMMAND *cmd);
void network_frame(int *keys_down);
void network_pause_game(int pause);
void network_config_read(FILE *fp, NETWORK_CONFIG *config);
void network_config_write(FILE *fp, NETWORK_CONFIG *config);
void network_config_edit(NETWORK_CONFIG *config);

#endif
