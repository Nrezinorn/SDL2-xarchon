/*--------------------------------------------------------------------------*/
/* computer controller configuration widget                                 */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_COMPUTER_IFACE_H
#define __MY_QT_COMPUTER_IFACE_H

#include <qtiface.h>

class QButtonGroup;
class QRadioButton;

/*--------------------------------------------------------------------------*/
/* QtComputerIface class                                                    */
/*--------------------------------------------------------------------------*/

class QtComputerIface : public QtIface
{
public:
    QtComputerIface(void *_config, int _side,
                    QWidget *parent = 0, const char *name = 0);

    virtual void updateConfig();
    
private:
    QButtonGroup *buttonBox;
    QRadioButton *easy, *medium, *hard;
};

#endif /* __MY_QT_COMPUTER_IFACE_H */
