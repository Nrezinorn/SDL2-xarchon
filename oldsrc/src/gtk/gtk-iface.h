/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  interface configuration                                   */
/*--------------------------------------------------------------------------*/

#ifndef __MY_GTK_IFACE_H
#define __MY_GTK_IFACE_H

#include <gtk-toolkit.h>
#include <computer.h>
#include <human.h>
#include <network.h>

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_computer_edit_config(COMPUTER_CONFIG *config);
void gtk_toolkit_human_edit_config(HUMAN_CONFIG *config);
void gtk_toolkit_network_edit_config(NETWORK_CONFIG *config);

#endif /* __MY_GTK_IFACE_H */
