#ifndef SHOWCOMMANDBUTTON_H
#define SHOWCOMMANDBUTTON_H

#include <QPushButton>
#include <process.h>
#include "commanddialog.h"

class ShowCommandButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ShowCommandButton(Process* processInit);
    ~ShowCommandButton();
    void updateShowCommandButton(Process* processInit);
    Process* process;
    CommandDialog* commandDialog;

public slots:
    void showCommandDialogSlot();
};

#endif // SHOWCOMMANDBUTTON_H
