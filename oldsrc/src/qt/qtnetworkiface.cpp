/*--------------------------------------------------------------------------*/
/* network controller configuration widget                                  */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qtnetworkiface.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>

#include <network.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/* QtNetworkIface::QtNetworkIface                                           */
/*--------------------------------------------------------------------------*/

QtNetworkIface::QtNetworkIface(void *_config, int _side,
                               QWidget *parent = 0, const char *name = 0)
    : QtIface(_config, _side, parent, name)
{
    QVBox *box = new QVBox(this);
    
    QString text("Network mode: ");
    if (side == 0)
        text.append("client");
    else
        text.append("host");
    QLabel *label = new QLabel(text, box);
    
    if (side == 0) {
        label = new QLabel("Address of host computer", box);
        char *contents = ((NETWORK_CONFIG *)config)->address;
        address = new QLineEdit(contents, box);
        address->setMaxLength(64);
    }

    label = new QLabel("Port Number", box);
    char contents[8];
    sprintf(contents, "%d", ((NETWORK_CONFIG *)config)->port);
    port = new QLineEdit(contents, box);
    port->setMaxLength(5);
    
    box->setMinimumSize(400, 150);
}

/*--------------------------------------------------------------------------*/
/* QtNetworkIface::updateConfig                                             */
/*--------------------------------------------------------------------------*/

void QtNetworkIface::updateConfig()
{
    if (side == 0)
        strcpy(((NETWORK_CONFIG *)config)->address,
               (const char *)address->text());

    ((NETWORK_CONFIG *)config)->port = atoi((const char *)port->text());
}
