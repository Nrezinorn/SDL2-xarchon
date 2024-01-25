/*--------------------------------------------------------------------------*/
/* players configuration dialog                                             */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_PLAYERS_DIALOG_H
#define __MY_QT_PLAYERS_DIALOG_H

#include <qdialog.h>

struct IFACE_PLAYER;
class QtPlayerWidget;
class QButtonGroup;

/*--------------------------------------------------------------------------*/
/* QtPlayersDialog class                                                    */
/*--------------------------------------------------------------------------*/

class QtPlayersDialog : public QDialog
{
    Q_OBJECT

public:
    QtPlayersDialog(QWidget *parent = 0, const char *name = 0);
    
    static QString getControllerName(int side);

public slots:
    void updateConfig();
    
private:
    static IFACE_PLAYER *findPlayer(int side);
    
    QtPlayerWidget *leftWidget, *rightWidget;
    QButtonGroup *firstBox;
};

#endif /* __MY_QT_PLAYERS_DIALOG_H */
