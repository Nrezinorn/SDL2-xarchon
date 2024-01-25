/*--------------------------------------------------------------------------*/
/* network controller configuration widget                                  */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_NETWORK_IFACE_H
#define __MY_QT_NETWORK_IFACE_H

#include <qtiface.h>

class QLineEdit;

/*--------------------------------------------------------------------------*/
/* QtNetworkIface class                                                     */
/*--------------------------------------------------------------------------*/

class QtNetworkIface : public QtIface
{
public:
    QtNetworkIface(void *_config, int _side,
                   QWidget *parent = 0, const char *name = 0);

    virtual void updateConfig();
    
private:
    QLineEdit *address, *port;
};

#endif /* __MY_QT_NETWORK_IFACE_H */
