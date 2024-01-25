/*--------------------------------------------------------------------------*/
/* computer controller configuration widget                                 */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qtcomputeriface.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <computer.h>
#include <string.h>

/*--------------------------------------------------------------------------*/
/* QtComputerIface::QtComputerIface                                         */
/*--------------------------------------------------------------------------*/

QtComputerIface::QtComputerIface(void *_config, int _side,
                                 QWidget *parent = 0, const char *name = 0)
    : QtIface(_config, _side, parent, name)
{
    buttonBox = new QButtonGroup(3, Qt::Vertical, this);
    buttonBox->setFrameStyle(QFrame::NoFrame);
    
    easy = new QRadioButton("Easy", buttonBox);
    medium = new QRadioButton("Medium", buttonBox);
    hard = new QRadioButton("Hard", buttonBox);
    
    QString rules = QString(((COMPUTER_CONFIG *)config)->rules);
    if (rules == "hard")
        buttonBox->setButton(2);
    else if (rules == "medium")
        buttonBox->setButton(1);
    else  /* rules == "easy", the default */
        buttonBox->setButton(0);
    
    buttonBox->setMinimumSize(300, 100);
    setMinimumSize(400, 150);
}

/*--------------------------------------------------------------------------*/
/* QtComputerIface::updateConfig                                            */
/*--------------------------------------------------------------------------*/

void QtComputerIface::updateConfig()
{
    char *rules = ((COMPUTER_CONFIG *)config)->rules;
    int id = buttonBox->id(buttonBox->selected());
    if (id == 2)
        strcpy(rules, "hard");
    else if (id == 1)
        strcpy(rules, "medium");
    else  /* id == 0, the default */
        strcpy(rules, "easy");
}
