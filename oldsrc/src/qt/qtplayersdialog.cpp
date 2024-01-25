/*--------------------------------------------------------------------------*/
/* players configuration dialog                                             */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <qtplayerwidget.h>
#include <qtplayersdialog.h>
#include <qtplayersdialog.moc>
#include <iface.h>
#include <main.h>

/*--------------------------------------------------------------------------*/
/* QtPlayersDialog::findPlayer                                              */
/*--------------------------------------------------------------------------*/

IFACE_PLAYER *QtPlayersDialog::findPlayer(int side)
{
    IFACE_CONFIG *config = iface_get_config();
    
    QString name = "?";
    if (side == 0)
        name = "Qt-Left";
    if (side == 1)
        name = "Qt-Right";
    
    IFACE_PLAYER *player = 0;
    for (player = (IFACE_PLAYER *)list_head(&config->players);
         player != NULL;
         player = (IFACE_PLAYER *)list_next(player)) {

        if (!strcmp(player->name, name))
            break;
    }
    
    if (!player) {
        player = iface_new_player();
        player->type = IFACE_HUMAN;
        strcpy(player->name, name);
    }
    
    if (side == 0)
        strcpy(config->light_name, name);
    else if (side == 1)
        strcpy(config->dark_name, name);

    return player;
}

/*--------------------------------------------------------------------------*/
/* QtPlayersDialog::QtPlayersDialog                                         */
/*--------------------------------------------------------------------------*/

QtPlayersDialog::QtPlayersDialog(QWidget *parent = 0, const char *name = 0)
    : QDialog(parent, name, TRUE)
{
    setCaption("X ARCHON Player Configuration");
    
    IFACE_PLAYER *leftData = findPlayer(0);
    IFACE_PLAYER *rightData = findPlayer(1);
    
    QGridLayout *grid = new QGridLayout(this, 4, 2, 10, 10);
    
    QLabel *leftLabel =
        new QLabel("Configure the Player on the Left:", this);
    QLabel *rightLabel =
        new QLabel("Configure the Player on the Right:", this);
    grid->addWidget(leftLabel, 0, 0);
    grid->addWidget(rightLabel, 0, 1);
    
    leftWidget = new QtPlayerWidget(leftData, 0, this);
    rightWidget = new QtPlayerWidget(rightData, 1, this);
    grid->addWidget(leftWidget, 1, 0);
    grid->addWidget(rightWidget, 1, 1);
    grid->setRowStretch(1, 1);
    
    firstBox = new QButtonGroup(2, Qt::Horizontal, this);
    firstBox->setFrameStyle(QFrame::Box + QFrame::Plain);
    firstBox->setMargin(2);
    
    QRadioButton *radio;
    radio = new QRadioButton("Left player moves first", firstBox);
    radio = new QRadioButton("Right player moves first", firstBox);
    
    IFACE_CONFIG *config = iface_get_config();
    firstBox->setButton(!config->light_first);
    grid->addMultiCellWidget(firstBox, 2, 2, 0, 1);
    
    QPushButton *ok = new QPushButton("&Ok", this);
    QPushButton *cancel = new QPushButton("&Cancel", this);
    connect(ok, SIGNAL(clicked()), this, SLOT(updateConfig()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    grid->addWidget(ok, 3, 0);
    grid->addWidget(cancel, 3, 1);
}

/*--------------------------------------------------------------------------*/
/* QtPlayersDialog::getControllerName                                       */
/*--------------------------------------------------------------------------*/

QString QtPlayersDialog::getControllerName(int side)
{
    IFACE_PLAYER *data = findPlayer(side);
    return QtPlayerWidget::getControllerName(data->type);
}

/*--------------------------------------------------------------------------*/
/* QtPlayersDialog::updateConfig                                            */
/*--------------------------------------------------------------------------*/

void QtPlayersDialog::updateConfig()
{
    if (leftWidget->isConfigOk() && rightWidget->isConfigOk()) {
        IFACE_CONFIG *config = iface_get_config();
        config->light_first = !firstBox->id(firstBox->selected());
        
        leftWidget->updateConfig();
        rightWidget->updateConfig();
        main_config_write();
        close();
    }
}
