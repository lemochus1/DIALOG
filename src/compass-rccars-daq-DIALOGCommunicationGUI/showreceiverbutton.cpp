#include "showreceiverbutton.h"

ShowReceiverButton::ShowReceiverButton(Process* processInit) :
    QPushButton("Show")
{
    process = processInit;
    receiverDialog = new ReceiverDialog(process);
    QObject::connect(this, &ShowReceiverButton::clicked, this, &ShowReceiverButton::showReceiverDialogSlot);
}

void ShowReceiverButton::showReceiverDialogSlot()
{
    receiverDialog->show();
    receiverDialog->activateWindow();
    if (process->servicesAsReceiver.count() > 0)
        receiverDialog->selectedServiceChangedSlot();
}

void ShowReceiverButton::updateShowReceiverButton(Process *processInit)
{
    process = processInit;
    receiverDialog->updateReceiverDialog(process);
}

ShowReceiverButton::~ShowReceiverButton()
{
    delete receiverDialog;
}
