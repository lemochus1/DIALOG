#include "showservicebutton.h"

ShowServiceButton::ShowServiceButton(Process* processInit) :
    QPushButton("Show")
{
    process = processInit;
    serviceDialog = new ServiceDialog(process);
    QObject::connect(this, &ShowServiceButton::clicked, this, &ShowServiceButton::showServiceDialogSlot);
}

void ShowServiceButton::showServiceDialogSlot()
{
    serviceDialog->show();
    serviceDialog->activateWindow();
    if (process->servicesAsSender.count() > 0)
        serviceDialog->selectedServiceChangedSlot();
}

void ShowServiceButton::updateShowServiceButton(Process *processInit)
{
    process = processInit;
    serviceDialog->updateServiceDialog(process);
}

ShowServiceButton::~ShowServiceButton()
{
    delete serviceDialog;
}
