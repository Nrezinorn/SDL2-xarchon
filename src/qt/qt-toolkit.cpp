/*--------------------------------------------------------------------------*/
/* qt toolkit                                                               */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdlib.h>

#include <qapplication.h>
#include <qmessagebox.h>

#include <qtguiwidget.h>
#include <qt-toolkit.h>
#include <toolkit.h>
#include <main.h>

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_close(void *_priv);
static void qt_toolkit_create_main_window(void *_priv);
static int qt_toolkit_is_toolkit_active(void *_priv);
static Display *qt_toolkit_get_xdisplay(void *_priv);
static Window qt_toolkit_get_xwindow(void *_priv);
void qt_toolkit_message_box(void *_priv, char *msg);
static void qt_toolkit_set_progressbar(void *_priv,
                                        char *msg, float progress);
static void qt_toolkit_set_sensitive_options(void *_priv, int game_active);
static void qt_toolkit_set_toolkit_active(void *_priv, int active);
static void qt_toolkit_set_visible_gui(void *_priv, int visible);
static void qt_toolkit_xflush(void *_priv);

/*--------------------------------------------------------------------------*/
/* qt_toolkit_init                                                          */
/*--------------------------------------------------------------------------*/

void *qt_toolkit_init(TOOLKIT *toolkit, int *argc, char **argv)
{
    int myargc = *argc;
    QApplication *app = new QApplication(myargc, argv);
    
    toolkit->close = qt_toolkit_close;
    
    toolkit->create_main_window    = qt_toolkit_create_main_window;
    toolkit->get_xdisplay          = qt_toolkit_get_xdisplay;
    toolkit->get_xwindow           = qt_toolkit_get_xwindow;
    toolkit->is_toolkit_active     = qt_toolkit_is_toolkit_active;
    toolkit->message_box           = qt_toolkit_message_box;
    toolkit->set_progressbar       = qt_toolkit_set_progressbar;
    toolkit->set_sensitive_options = qt_toolkit_set_sensitive_options;
    toolkit->set_toolkit_active    = qt_toolkit_set_toolkit_active; 
    toolkit->set_visible_gui       = qt_toolkit_set_visible_gui;
    toolkit->xflush                = qt_toolkit_xflush;
    
    return app;
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_close                                                         */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_close(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    if (guiwij)
        delete guiwij;
    delete app;
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_create_main_window                                            */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_create_main_window(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    
    QtGuiWidget *guiwij = new QtGuiWidget(NULL, "QtGui");
    app->setMainWidget(guiwij);
    guiwij->resize(CANVAS_WIDTH, CANVAS_HEIGHT);
    guiwij->show();

    app->processEvents();
    app->flushX();

}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_is_toolkit_active                                             */
/*--------------------------------------------------------------------------*/

static int qt_toolkit_is_toolkit_active(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    return app->loopLevel() > 0;
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_get_xdisplay                                                  */
/*--------------------------------------------------------------------------*/

static Display *qt_toolkit_get_xdisplay(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    return guiwij->getXDisplay();
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_get_xwindow                                                   */
/*--------------------------------------------------------------------------*/

static Window qt_toolkit_get_xwindow(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    return guiwij->getXWindow();
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_message_box                                                   */
/*--------------------------------------------------------------------------*/

void qt_toolkit_message_box(void *_priv, char *msg)
{
    QMessageBox box(QString("X ARCHON Message"), QString(msg),
                    QMessageBox::Information,
                    QMessageBox::Ok,
                    QMessageBox::NoButton, QMessageBox::NoButton);
    box.exec();
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_set_progressbar                                               */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_set_progressbar(void *_priv,
                                       char *msg, float progress)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    guiwij->setProgressBar(msg, progress);
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_set_sensitive_options                                         */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_set_sensitive_options(void *_priv, int game_active)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    guiwij->setSensitiveOptions(game_active);
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_set_toolkit_active                                            */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_set_toolkit_active(void *_priv, int active)
{
    QApplication *app = (QApplication *)_priv;
    if (active)
        app->exec();
    else
        app->quit();
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_set_visible_gui                                               */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_set_visible_gui(void *_priv, int visible)
{
    QApplication *app = (QApplication *)_priv;
    QtGuiWidget *guiwij = (QtGuiWidget *)app->mainWidget();
    guiwij->setVisible(visible);
}

/*--------------------------------------------------------------------------*/
/* qt_toolkit_xflush                                                        */
/*--------------------------------------------------------------------------*/

static void qt_toolkit_xflush(void *_priv)
{
    QApplication *app = (QApplication *)_priv;
    app->flushX();
}
