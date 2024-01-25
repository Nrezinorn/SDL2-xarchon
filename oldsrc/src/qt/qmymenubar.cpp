/*--------------------------------------------------------------------------*/
/* my menu bar widget                                                       */
/*--------------------------------------------------------------------------*/

#include <qmymenubar.h>

/*--------------------------------------------------------------------------*/
/* class QMyMenuBarItem                                                     */
/*--------------------------------------------------------------------------*/

class QMyMenuBarItem
{
public:
    QMyMenuBarItem(QPopupMenu *_popup, const int _id)
        { popup = _popup; id = _id; }
    
    void setEnabled(bool enable)
        { popup->setItemEnabled(id, enable); }
    
    void setChecked(bool check)
        { popup->setItemChecked(id, check); }
    
    bool isEnabled()
        { return popup->isItemEnabled(id); }
    
    bool isChecked()
        { return popup->isItemChecked(id); }
    
private:
    QPopupMenu *popup;
    int id;
};

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::QMyMenuBar                                                   */
/*--------------------------------------------------------------------------*/

QMyMenuBar::QMyMenuBar(QWidget *parent = 0, const char *name = 0)
    : QMenuBar(parent, name)
{
    dict.setAutoDelete(TRUE);
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::insertItem                                                   */
/*--------------------------------------------------------------------------*/

int QMyMenuBar::insertItem(const QString &text, QPopupMenu *popup,
                           int id = -1, int index = -1)
{
   QMenuBar::insertItem(text, popup, id, index);
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::insertItem                                                   */
/*--------------------------------------------------------------------------*/

int QMyMenuBar::insertItem(const char *name, QPopupMenu *popup,
                           const QString &text,
                           const QObject *receiver, const char *member,
                           int accel = 0, int id = -1, int index = -1)
{
    int the_id = popup->insertItem(text, receiver, member, accel, id, index);
    QMyMenuBarItem *item = new QMyMenuBarItem(popup, the_id);
    dict.insert(name, item);
    return the_id;
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::setEnabled                                                   */
/*--------------------------------------------------------------------------*/

void QMyMenuBar::setEnabled(const char *name, bool enable)
{
    QMyMenuBarItem *item = dict.find(name);
    if (item)
        item->setEnabled(enable);
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::setChecked                                                   */
/*--------------------------------------------------------------------------*/

void QMyMenuBar::setChecked(const char *name, bool check)
{
    QMyMenuBarItem *item = dict.find(name);
    if (item)
        item->setChecked(check);
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::isEnabled                                                    */
/*--------------------------------------------------------------------------*/

bool QMyMenuBar::isEnabled(const char *name)
{
    QMyMenuBarItem *item = dict.find(name);
    if (item)
        return item->isEnabled();
    return FALSE;
}

/*--------------------------------------------------------------------------*/
/* QMyMenuBar::isChecked                                                    */
/*--------------------------------------------------------------------------*/

bool QMyMenuBar::isChecked(const char *name)
{
    QMyMenuBarItem *item = dict.find(name);
    if (item)
        return item->isChecked();
    return FALSE;
}
