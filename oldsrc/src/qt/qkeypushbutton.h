/*--------------------------------------------------------------------------*/
/* a push button that is used to configure a key                            */
/*--------------------------------------------------------------------------*/

#ifndef __MY_Q_KEY_PUSH_BUTTON_H
#define __MY_Q_KEY_PUSH_BUTTON_H

#include <qpushbutton.h>

/*--------------------------------------------------------------------------*/
/* QKeyPushButton class                                                     */
/*--------------------------------------------------------------------------*/

class QKeyPushButton : public QPushButton
{
    Q_OBJECT
    
public:
    QKeyPushButton(int _id, QString &_descr, unsigned long _key,
                   QWidget *parent = 0, const char *name = 0);
    
    void setKey(unsigned long _key);
    
    unsigned long getKey()
        { return key; };
    
protected:
    void updateText();
    
private slots:
    void inputKey();
    
signals:
    void newKey(int id, unsigned long key);
    
private:
    int id;
    QString descr;
    unsigned long key;
};

#endif /* __MY_Q_KEY_PUSH_BUTTON_H */
