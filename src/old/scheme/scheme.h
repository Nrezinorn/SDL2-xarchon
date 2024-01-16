/*--------------------------------------------------------------------------*/
/* scheme interface support                                                 */
/*--------------------------------------------------------------------------*/

#ifndef __SCHEME_H
#define __SCHEME_H

#include "iface.h"

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void scheme_init(int argc, char *argv[]);
void *scheme_start(int which, char *name);
void scheme_turn(void *data, int mode, COMMAND *cmd);

#endif
