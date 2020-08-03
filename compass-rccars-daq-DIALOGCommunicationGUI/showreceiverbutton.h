#ifndef SHOWRECEIVERBUTTON_H
#define SHOWRECEIVERBUTTON_H

#include <QPushButton>
#include <process.h>
#include "receiverdialog.h"

class ShowReceiverButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ShowReceiverButton(Process* processInit);
    ~ShowReceiverButton();
    void updateShowReceiverButton(Process* processInit);
    Process* process;
    ReceiverDialog* receiverDialog;

public slots:
    void showReceiverDialogSlot();
};

#endif // SHOWRECEIVERBUTTON_H
