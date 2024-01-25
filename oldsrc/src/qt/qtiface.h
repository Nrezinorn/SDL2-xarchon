/*--------------------------------------------------------------------------*/
/* abstract controller configuration widget                                 */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_IFACE_H
#define __MY_QT_IFACE_H

#include <qwidget.h>

/*--------------------------------------------------------------------------*/
/* QtIface class                                                            */
/*--------------------------------------------------------------------------*/

class QtIface : public QWidget
{
    Q_OBJECT
    
public:
    QtIface(void *_config, int _side,
            QWidget *parent = 0, const char *name = 0)
        : QWidget(parent, name)
        { config = _config; side = _side; };
    
    virtual bool isConfigOk()
        { return true; }
    
    virtual void updateConfig() = 0;
    
protected:
    void *config;
    int side;
};

#endif /* __MY_QT_IFACE_H */
