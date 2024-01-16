/*--------------------------------------------------------------------------*/
/* human controller configuration widget                                    */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_HUMAN_IFACE_H
#define __MY_QT_HUMAN_IFACE_H

#include <qtiface.h>

class QKeyPushButton;

/*--------------------------------------------------------------------------*/
/* QtHumanIface class                                                       */
/*--------------------------------------------------------------------------*/

class QtHumanIface : public QtIface
{
    Q_OBJECT

public:
    QtHumanIface(void *_config, int _side,
                 QWidget *parent = 0, const char *name = 0);
    ~QtHumanIface();
    
    virtual bool isConfigOk();
    virtual void updateConfig();
    
public slots:
    virtual void newKey(int id, unsigned long key);
    virtual void newNonKeyboard(int index);
    
private:
    QKeyPushButton **buttons;
/*    unsigned long *keys; */
    int non_keyboard;                   /* -1 = keyboard only */
                                        /* >= 0 && < JOY_MAX_DEV = joystick */
                                        /* >= JOY_MAX_DEV = mouse */
};

#endif /* __MY_QT_HUMAN_IFACE_H */
