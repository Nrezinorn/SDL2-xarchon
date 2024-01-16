/*--------------------------------------------------------------------------*/
/* player configuration widget                                              */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_PLAYER_WIDGET_H
#define __MY_QT_PLAYER_WIDGET_H

#include <qvbox.h>

#include <iface.h>

class QButtonGroup;
class QWidgetStack;
class QtIface;

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget class                                                     */
/*--------------------------------------------------------------------------*/

class QtPlayerWidget : public QVBox
{
public:
    QtPlayerWidget(IFACE_PLAYER *data, int side,
                   QWidget *parent = 0, const char *name = 0);
    ~QtPlayerWidget();

    bool isConfigOk();
    void updateConfig();

    static QString getControllerName(int type);
    
private:
    IFACE_PLAYER *config;
    QtIface *ifaces[4];             /* human, computer, network */
                                    /* ifaces[0] is unused! */
    QButtonGroup *typesBox;
    QWidgetStack *stack;
};

#endif /* __MY_QT_PLAYER_WIDGET_H */
