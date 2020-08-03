#include "showcommandbutton.h"

ShowCommandButton::ShowCommandButton(Process* processInit) :
    QPushButton("Show")
{
    process = processInit;
    commandDialog = new CommandDialog(process);
    QObject::connect(this, &ShowCommandButton::clicked, this, &ShowCommandButton::showCommandDialogSlot);
}

void ShowCommandButton::showCommandDialogSlot()
{
    commandDialog->show();
    commandDialog->activateWindow();
    if (process->commands.count() > 0)
        commandDialog->selectedCommandChangedSlot();
}

void ShowCommandButton::updateShowCommandButton(Process *processInit)
{
    process = processInit;
    commandDialog->updateCommandDialog(process);
}

ShowCommandButton::~ShowCommandButton()
{
    delete commandDialog;
}
