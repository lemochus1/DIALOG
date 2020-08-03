#ifndef SHOWSERVICEBUTTON_H
#define SHOWSERVICEBUTTON_H

#include <QPushButton>
#include <process.h>
#include "servicedialog.h"

class ShowServiceButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ShowServiceButton(Process* processInit);
    ~ShowServiceButton();
    void updateShowServiceButton(Process* processInit);
    Process* process;
    ServiceDialog* serviceDialog;

public slots:
    void showServiceDialogSlot();
};

#endif // SHOWSERVICEBUTTON_H
