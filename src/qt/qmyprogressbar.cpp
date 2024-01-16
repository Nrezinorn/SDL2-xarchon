/*--------------------------------------------------------------------------*/
/* my progress bar widget                                                   */
/*--------------------------------------------------------------------------*/

#include "qmyprogressbar.h"

/*--------------------------------------------------------------------------*/
/* QMyProgressBar::QMyProgressBar                                           */
/*--------------------------------------------------------------------------*/

QMyProgressBar::QMyProgressBar(QWidget *parent = 0, const char *name = 0)
    : QProgressBar(parent, name)
{
    setCenterIndicator(TRUE);
}

/*--------------------------------------------------------------------------*/
/* QMyProgressBar::setMessage                                               */
/*--------------------------------------------------------------------------*/

void QMyProgressBar::setMessage(char *text)
{
    message = QString(text);
}

/*--------------------------------------------------------------------------*/
/* QMyProgressBar::setIndicator                                             */
/*--------------------------------------------------------------------------*/

bool QMyProgressBar::setIndicator(QString &indicator, int progress, int total)
{
    indicator = message;
    return TRUE;
}
