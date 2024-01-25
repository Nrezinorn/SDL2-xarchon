/*--------------------------------------------------------------------------*/
/* main gui widget                                                          */
/*--------------------------------------------------------------------------*/

#ifndef __MY_QT_GUI_WIDGET_H
#define __MY_QT_GUI_WIDGET_H

#include <qmainwindow.h>
#include <X11/Xlib.h>

class QMyMenuBar;
class QPixmap;
class QLabel;
class QStatusBar;
class QMyProgressBar;

/*--------------------------------------------------------------------------*/
/* QtGuiWidget class                                                        */
/*--------------------------------------------------------------------------*/

class QtGuiWidget : public QMainWindow
{
    Q_OBJECT

public:
    QtGuiWidget(QWidget *parent = 0, const char *name = 0);

    Display *getXDisplay();
    Window getXWindow();
    void setProgressBar(char *msg, float progress);
    void setSensitiveOptions(bool game_active);
    void setVisible(bool visible);
    
    virtual bool close(bool alsoDelete);
    
private slots:
    void gameArchon();
    void gameAdept();
    void gameUnpause();
    void gameStop();
    void settingsPlayers();
    void settingsTheme();
    void settingsSound();
    void helpAbout();
    
private:
    void createMenu();
    
    QMyMenuBar *menubar;
    QPixmap *logo_pixmap;
    QLabel *logo;
    QStatusBar *statusbar;
    QMyProgressBar *myprogressbar;
};

#endif /* __MY_QT_GUI_WIDGET_H */
