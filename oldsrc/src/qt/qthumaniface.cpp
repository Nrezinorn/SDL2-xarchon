/*--------------------------------------------------------------------------*/
/* human controller configuration widget                                    */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qthumaniface.h>
#include <qthumaniface.moc>
#include <qlayout.h>
#include <qkeypushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <actors.h>
#include <human.h>
#include <toolkit.h>

/*--------------------------------------------------------------------------*/
/* Variables                                                                */
/*--------------------------------------------------------------------------*/

static char *qt_direction_name[] = {
   "Fire",
   "Up",
   "Dn",
   "Lt",
   "Rt",
   "UpLt",
   "DnLt",
   "UpRt",
   "DnRt"
};

static int qt_buttons_order[] = {
    STATE_MOVE_UP_LEFT,
    STATE_MOVE_UP,
    STATE_MOVE_UP_RIGHT,
    STATE_MOVE_LEFT,
    STATE_FIRE,
    STATE_MOVE_RIGHT,
    STATE_MOVE_DOWN_LEFT,
    STATE_MOVE_DOWN,
    STATE_MOVE_DOWN_RIGHT
};

/*--------------------------------------------------------------------------*/
/* QtHumanIface::QtHumanIface                                               */
/*--------------------------------------------------------------------------*/

QtHumanIface::QtHumanIface(void *_config, int _side,
                           QWidget *parent = 0, const char *name = 0)
    : QtIface(_config, _side, parent, name)
{
    QGridLayout *grid = new QGridLayout(this, 5, 3, 5, 5);
    
    buttons = new QKeyPushButton *[STATE_MOVE_COUNT];
    
    for (int i = 0; i < STATE_MOVE_COUNT; i++) {
        int j = qt_buttons_order[i];
        QString descr = qt_direction_name[j];
        unsigned long key = ((HUMAN_CONFIG *)config)->keys[side][j];
        QKeyPushButton *button = new QKeyPushButton(j, descr, key, this);
        connect(button, SIGNAL(newKey(int, unsigned long)),
                this, SLOT(newKey(int, unsigned long)));
        grid->addWidget(button, (i / 3), (i % 3));

        buttons[j] = button;
    }

    non_keyboard = ((HUMAN_CONFIG *)config)->non_keyboard;
    
    QLabel *label = new QLabel("Additional joystick or mouse control:", this);
    grid->addMultiCellWidget(label, 3, 3, 0, 2);
    
    QComboBox *combo = new QComboBox(this);
    combo->insertItem("Don\'t use a joystick or mouse");
    combo->insertItem("Use the first joystick");
    combo->insertItem("Use the second joystick");
    combo->insertItem("Use the third joystick");
    combo->insertItem("Use the fourth joystick");
    combo->insertItem("Use mouse, motion is relative");
    combo->insertItem("Use mouse, motion is absolute");
    combo->setEditable(false);
    combo->setCurrentItem(non_keyboard + 1);
    connect(combo, SIGNAL(activated(int)), this, SLOT(newNonKeyboard(int)));
    grid->addMultiCellWidget(combo, 4, 4, 0, 2);
    
    setMinimumSize(400, 150);
}

/*--------------------------------------------------------------------------*/
/* QtHumanIface::~QtHumanIface                                              */
/*--------------------------------------------------------------------------*/

QtHumanIface::~QtHumanIface()
{
    for (int i = 0; i < STATE_MOVE_COUNT; i++)
        delete buttons[i];
    delete buttons;
}

/*--------------------------------------------------------------------------*/
/* QtHumanIface::newKey                                                     */
/*--------------------------------------------------------------------------*/

void QtHumanIface::newKey(int id, unsigned long key)
{
    for (int i = 0; i < STATE_MOVE_COUNT; i++)
        if (i != id) {
            QKeyPushButton *button = buttons[i];
            if (button->getKey() == key)
                button->setKey(0);
        }
}

/*--------------------------------------------------------------------------*/
/* QtHumanIface::newNonKeyboard                                             */
/*--------------------------------------------------------------------------*/

void QtHumanIface::newNonKeyboard(int index)
{
    non_keyboard = index - 1;
}

/*--------------------------------------------------------------------------*/
/* QtHumanIface::isConfigOk                                                 */
/*--------------------------------------------------------------------------*/

bool QtHumanIface::isConfigOk()
{
    if (non_keyboard != -1 && non_keyboard < HUMAN_JOY_MAX_DEV) {
#ifdef HAVE_LINUX_JOYSTICK_H
        if (!human_joystick_init(non_keyboard)) {
#endif
            toolkit_message_box("Joystick not available");
            return false;
#ifdef HAVE_LINUX_JOYSTICK_H
        }
#endif
    }
    
    return true;
}

/*--------------------------------------------------------------------------*/
/* QtHumanIface::updateConfig                                               */
/*--------------------------------------------------------------------------*/

void QtHumanIface::updateConfig()
{
    for (int i = 0; i < STATE_MOVE_COUNT; i++) {
        int j = qt_buttons_order[i];
        QKeyPushButton *button = buttons[j];
        ((HUMAN_CONFIG *)config)->keys[side][j] = button->getKey();
    }
    ((HUMAN_CONFIG *)config)->non_keyboard = non_keyboard;
}
