/*--------------------------------------------------------------------------*/
/* my menu bar widget                                                       */
/*--------------------------------------------------------------------------*/

#ifndef __MY_Q_MY_MENU_BAR_H
#define __MY_Q_MY_MENU_BAR_H

#include <qdict.h>
#include <qmenubar.h>

class QMyMenuBarItem;

/*--------------------------------------------------------------------------*/
/* QMyMenuBar class                                                         */
/*--------------------------------------------------------------------------*/

class QMyMenuBar : public QMenuBar
{
public:
    QMyMenuBar(QWidget *parent = 0, const char *name = 0);
    
    int insertItem(const QString &text, QPopupMenu *popup,
                   int id = -1, int index = -1);
    int insertItem(const char *name, QPopupMenu *popup,
                   const QString &text,
                   const QObject *receiver, const char *member,
                   int accel = 0, int id = -1, int index = -1);
    void setEnabled(const char *name, bool enable);
    void setChecked(const char *name, bool check);
    bool isEnabled(const char *name);
    bool isChecked(const char *name);
    
private:
    QDict<QMyMenuBarItem> dict;
};

#endif /* __MY_Q_MY_MENU_BAR_H */
