/*--------------------------------------------------------------------------*/
/* main gui widget                                                          */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qmymenubar.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmyprogressbar.h>
#include <qmessagebox.h>

#include <qtguiwidget.h>
#include <qtguiwidget.moc>
#include <qtplayersdialog.h>
#include <main.h>

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::createMenu                                                  */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::createMenu()
{
    QPopupMenu *popup;
    
    menubar = new QMyMenuBar(this, "Menubar");
    
    popup = new QPopupMenu(this, "Game");
    menubar->insertItem("&Game", popup);

    menubar->insertItem("gameArchon", popup, "New Game of Archon",
                        this, SLOT(gameArchon()));
    menubar->insertItem("gameAdept", popup, "New Game of Adept",
                        this, SLOT(gameAdept()));
    menubar->insertItem("gameUnpause", popup, "Un&pause",
                        this, SLOT(gameUnpause()), Qt::Key_Escape);
    menubar->insertItem("gameStop", popup, "&Stop",
                        this, SLOT(gameStop()), Qt::Key_F12);
    popup->insertSeparator();
    menubar->insertItem("gameQuit", popup, "&Quit",
                        this, SLOT(close()), Qt::CTRL | Qt::Key_Q);

    popup = new QPopupMenu(this, "Settings");
    menubar->insertItem("&Settings", popup);

    menubar->insertItem("settingsPlayers", popup, "Configure &Players...",
                        this, SLOT(settingsPlayers()));
    menubar->insertItem("settingsTheme", popup, "Select &Theme...",
                        this, SLOT(settingsTheme()));
    menubar->insertItem("settingsSound", popup, "Toggle &Sound",
                        this, SLOT(settingsSound()), Qt::Key_F11);
    menubar->setChecked("settingsSound", TRUE);

    popup = new QPopupMenu(this, "Help");
    menubar->insertItem("&Help", popup);
    
    menubar->insertItem("helpAbout", popup, "&About...",
                        this, SLOT(helpAbout()));

    setSensitiveOptions(FALSE);
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::QtGuiWidget                                                 */
/*--------------------------------------------------------------------------*/

QtGuiWidget::QtGuiWidget(QWidget *parent = 0, const char *name = 0)
    : QMainWindow(parent, name)
{
    setCaption("X ARCHON");
    
    createMenu();
    
    logo_pixmap = new QPixmap(QString(DATADIR "/logo.xpm"));
    logo = new QLabel(this);
    logo->setPixmap(*logo_pixmap);
    logo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::ExpandTabs);
    setCentralWidget(logo);
    
    statusbar = statusBar();
    statusbar->setSizeGripEnabled(FALSE);
    myprogressbar = new QMyProgressBar(statusbar);
    statusbar->addWidget(myprogressbar, TRUE, TRUE);
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::getXDisplay                                                 */
/*--------------------------------------------------------------------------*/

Display *QtGuiWidget::getXDisplay()
{
    return x11Display();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::getXWindow                                                  */
/*--------------------------------------------------------------------------*/

Window QtGuiWidget::getXWindow()
{
    return handle();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::setProgressBar                                              */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::setProgressBar(char *msg, float progress)
{
    myprogressbar->setMessage(msg);
    if (progress == 1.0)
        myprogressbar->reset();
    myprogressbar->setProgress((int)(progress * 100));
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::setSensitiveOptions                                         */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::setSensitiveOptions(bool game_active)
{
    menubar->setEnabled("gameArchon",      !game_active);
    menubar->setEnabled("gameAdept",       !game_active);
    menubar->setEnabled("gameStop",         game_active);
    menubar->setEnabled("settingsPlayers", !game_active);
    menubar->setEnabled("settingsTheme",   !game_active);
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::setVisible                                                  */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::setVisible(bool visible)
{
    if (visible) {
        menubar->show();
        logo->show();
        statusbar->show();
    } else {
        menubar->hide();
        logo->hide();
        statusbar->hide();
    }
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::close                                                       */
/*--------------------------------------------------------------------------*/

bool QtGuiWidget::close(bool alsoDelete)
{
    bool ret = QMainWindow::close(alsoDelete);
    if (ret)
        main_destroy_event();
    return ret;
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::gameArchon                                                  */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::gameArchon()
{
    main_new_game(GAME_ARCHON);
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::gameAdept                                                   */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::gameAdept()
{
    main_new_game(GAME_ADEPT);
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::gameUnpause                                                 */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::gameUnpause()
{
    main_game_unpause();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::gameStop                                                    */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::gameStop()
{
    main_game_stop();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::settingsPlayers                                             */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::settingsPlayers()
{
    QtPlayersDialog dialog(this);
    dialog.exec();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::settingsTheme                                               */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::settingsTheme()
{
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::settingsSound                                               */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::settingsSound()
{
    bool check = menubar->isChecked("settingsSound");
    menubar->setChecked("settingsSound", !check);
    main_toggle_sound();
}

/*--------------------------------------------------------------------------*/
/* QtGuiWidget::helpAbout                                                   */
/*--------------------------------------------------------------------------*/

void QtGuiWidget::helpAbout()
{
    QString msg = QString(
        "Based on an original game by Anne Westfall, Jon Freeman and"
        " Paul Reiche, III.\n"
        "\n"
        "Written by Ronen Tzur <rtzur@shani.net> with help from:\n"
        "   Dan Hursh <hursh@sparc.isl.net>\n"
        "   Shai Roitman <shairoi@ibm.net>\n"
        "   Robert Paige Rendell <rendell@cs.monash.edu.au>\n"
        "   Mike Ciul <mike@eyeballsun.org>\n"
        "\n"
        "Graphics by:\n"
        "   Dan LaPine <d-lapine@cecer.army.mil>\n"
        "   Matt Kimball <mkimball@xmission.com>\n"
        "   Mark Shoulson <mark@kli.org>\n"
        "   Joerg Osarek <joerg@osarek.de>\n"
        "\n"
        "Visit the homepage at http://xarchon.seul.org.\n"
        "We thank the SEUL project (http://www.seul.org) for hosting us.\n"
        "\n"
        "Have fun!\n");
    
    QMessageBox box(QString("About X ARCHON"), QString(msg),
                    QMessageBox::Information,
                    QMessageBox::Ok,
                    QMessageBox::NoButton, QMessageBox::NoButton,
                    this);
    box.exec();
}
