/*--------------------------------------------------------------------------*/
/* player configuration widget                                              */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwidgetstack.h>

#include <qtplayerwidget.h>
#include <qtiface.h>
#include <qthumaniface.h>
#include <qtcomputeriface.h>
#include <qtnetworkiface.h>
#include <iface.h>

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget::QtPlayerWidget                                           */
/*--------------------------------------------------------------------------*/

QtPlayerWidget::QtPlayerWidget(IFACE_PLAYER *data, int side,
                               QWidget *parent = 0, const char *name = 0)
    : QVBox(parent, name)
{
    setFrameStyle(Box + Plain);
    setSpacing(5);
    setMargin(5);
    
    typesBox = new QButtonGroup(3, Qt::Vertical,
                                "Who or what controls this player?", this);
    typesBox->setFrameStyle(QFrame::Box + QFrame::Plain);

    QRadioButton *radio;
    radio = new QRadioButton(getControllerName(1), typesBox);
    radio = new QRadioButton(getControllerName(2), typesBox);
    radio = new QRadioButton(getControllerName(3), typesBox);

    config = data;
    int type = config->type - 1;
    typesBox->setButton(type);
    
    stack = new QWidgetStack(this);
    stack->setFrameStyle(Box + Plain);
    stack->setMargin(5);

    QtIface *iface;
    iface = (QtIface *)new QtHumanIface(data->human, side, stack);
    stack->addWidget(iface, 0);
    iface = (QtIface *)new QtComputerIface(data->computer, side, stack);
    stack->addWidget(iface, 1);
    iface = (QtIface *)new QtNetworkIface(data->network, side, stack);
    stack->addWidget(iface, 2);
    
    stack->raiseWidget(type);
    connect(typesBox, SIGNAL(clicked(int)), stack, SLOT(raiseWidget(int)));
}

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget::~QtPlayerWidget                                          */
/*--------------------------------------------------------------------------*/

QtPlayerWidget::~QtPlayerWidget()
{
    int i = 0;
    while (1) {
        QtIface *iface = (QtIface *)(stack->widget(i));
        if (!iface)
            break;
        delete iface;
        i++;
    }
}

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget::getControllerName                                        */
/*--------------------------------------------------------------------------*/

QString QtPlayerWidget::getControllerName(int type)
{
    if (type == 1)
        return QString("a person");
    if (type == 2)
        return QString("the computer");
    if (type == 3)
        return QString("network input");
    return QString("unknown controller");
}

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget::isConfigOk                                               */
/*--------------------------------------------------------------------------*/

bool QtPlayerWidget::isConfigOk(void)
{
    QWidget *visible = stack->visibleWidget();
    bool ok = (visible != 0);
    
    int i = 0;
    while (1) {
        QtIface *iface = (QtIface *)(stack->widget(i));
        if (!iface)
            break;
        stack->raiseWidget(i);
        if (!iface->isConfigOk())
            ok = false;
        i++;
    }

    stack->raiseWidget(visible);
    return ok;
}

/*--------------------------------------------------------------------------*/
/* QtPlayerWidget::updateConfig                                             */
/*--------------------------------------------------------------------------*/

void QtPlayerWidget::updateConfig(void)
{
    config->type = typesBox->id(typesBox->selected()) + 1;
    
    int i = 0;
    while (1) {
        QtIface *iface = (QtIface *)(stack->widget(i));
        if (!iface)
            break;
        iface->updateConfig();
        i++;
    }
}
