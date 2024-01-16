/*--------------------------------------------------------------------------*/
/* a push button that is used to configure a key                            */
/*--------------------------------------------------------------------------*/

#include <qkeypushbutton.h>
#include <qkeypushbutton.moc>
#include <qaccel.h>

#include <X11/Xlib.h>

/*--------------------------------------------------------------------------*/
/* QKeyPushButton::QKeyPushButton                                           */
/*--------------------------------------------------------------------------*/

QKeyPushButton::QKeyPushButton(int _id, QString &_descr, unsigned long _key,
                               QWidget *parent = 0, const char *name = 0)
    : QPushButton(parent, name)
{
    id = _id;
    descr = _descr;
    key = _key;
    connect(this, SIGNAL(clicked()), this, SLOT(inputKey()));
    updateText();
    setFocusPolicy(QWidget::StrongFocus);
}

/*--------------------------------------------------------------------------*/
/* QKeyPushButton::setKey                                                   */
/*--------------------------------------------------------------------------*/

void QKeyPushButton::setKey(unsigned long _key)
{
    key = _key;
    updateText();
}

/*--------------------------------------------------------------------------*/
/* QKeyPushButton::updateText                                               */
/*--------------------------------------------------------------------------*/

void QKeyPushButton::updateText(void)
{
    QString text(descr);
    text.append(": ");
    text.append(XKeysymToString(key));
    setText(text);
}

/*--------------------------------------------------------------------------*/
/* QKeyPushButton::inputKey                                                 */
/*--------------------------------------------------------------------------*/

void QKeyPushButton::inputKey(void)
{
    setText("Press Key Now");
    qApp->processEvents();
    qApp->flushX();
    
    bool done = false;
    while (!done) {
        XEvent ev;
        XNextEvent(x11Display(), &ev);
        
        switch (ev.type) {
            case KeyRelease:
                key = XLookupKeysym(&ev.xkey, 0);
                done = true;
                break;
            
            case Expose:
            case GraphicsExpose:
            case NoExpose:
            case VisibilityNotify:
                XPutBackEvent(x11Display(), &ev);
                qApp->processOneEvent();
                break;
            
            default:
                /* ignore event */
                break;
        }
    }
    
    updateText();
    emit newKey(id, key);
}
