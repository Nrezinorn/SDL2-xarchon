/*--------------------------------------------------------------------------*/
/* my progress bar widget                                                   */
/*--------------------------------------------------------------------------*/

#ifndef __MY_Q_MY_PROGRESS_BAR_H
#define __MY_Q_MY_PROGRESS_BAR_H

#include <qprogressbar.h>

/*--------------------------------------------------------------------------*/
/* QMyProgressBar class                                                     */
/*--------------------------------------------------------------------------*/

class QMyProgressBar : public QProgressBar
{
public:
    QMyProgressBar(QWidget *parent = 0, const char *name = 0);
    
    void setMessage(char *text);
    
protected:
    virtual bool setIndicator(QString &indicator, int progress, int total);
    
private:
    QString message;
};

#endif /* __MY_Q_MY_PROGRESS_BAR_H */
