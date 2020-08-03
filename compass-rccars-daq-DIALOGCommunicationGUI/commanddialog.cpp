#include "commanddialog.h"
#include "ui_commanddialog.h"

CommandDialog::CommandDialog(Process* processInit, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandDialog)
{
    process = processInit;
    ui->setupUi(this);

    selectedCommandName = "";
    selectedCommand = NULL;
    isCommandRegistered = false;

    this->setFixedSize(900, 540);
    this->setWindowTitle("Commands of process " + process->processName + " running on address " + process->processAddress + " on port " + QString::number(process->processPort) + " with PID " + QString::number(process->processPID));

    ui->commandListLabel->setGeometry(10,10,298,20);
    ui->commandList->setGeometry(10,30,298,200);
    ui->commandList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->commandMessageLabel->setGeometry(318,10,572,20);
    ui->commandMessage->setGeometry(318,30,572,163);

    ui->sendCommandButton->setGeometry(774,203,121,27);

    ui->commandOutputLabel->setGeometry(10,240,592,20);
    ui->commandOutput->setGeometry(10,260,880,233);
    ui->commandOutput->setReadOnly(true);

    ui->receiverAllCommands->setGeometry(602,240,288,20);

    ui->startListeningButton->setGeometry(660,503,110,27);
    ui->stopListeningButton->setGeometry(780,503,110,27);
    ui->stopListeningButton->setEnabled(false);

    if (process->commands.count() > 0)
    {
        foreach (Command* command, process->commands)
            ui->commandList->addItem(command->commandName);
        ui->commandList->sortItems(Qt::AscendingOrder);
        ui->commandList->setCurrentRow(0);
    }
    else
    {
        noCommandAvailable();
    }

    QObject::connect(ui->commandList, &QListWidget::itemSelectionChanged, this, &CommandDialog::selectedCommandChangedSlot);
    QObject::connect(ui->sendCommandButton, &QPushButton::clicked, this, &CommandDialog::sendCommandMessageSlot);
    QObject::connect(ui->receiverAllCommands, &QCheckBox::clicked, this, &CommandDialog::receiverAllCommandsSlot);
    QObject::connect(ui->startListeningButton, &QPushButton::clicked, this, &CommandDialog::startListeningSlot);
    QObject::connect(ui->stopListeningButton, &QPushButton::clicked, this, &CommandDialog::stopListeningSlot);
}

void CommandDialog::selectedCommandChangedSlot()
{
    if (process->commands.count() > 0)
    {
        selectedCommandName = ui->commandList->selectedItems().first()->text();

        foreach (Command* command, process->commands) {
            if(command->commandName == selectedCommandName)
            {
                selectedCommand = command;
                break;
            }
        }

        commandsAvailable();
    }
    else
    {
        noCommandAvailable();
    }
}

void CommandDialog::updateCommandDialog(Process *processInit)
{
    process = processInit;

    if (process->commands.count() > 0)
    {
        bool isSelectedCommandStillAvailable = false;
        if(ui->commandList->count() > 0)
        {
            foreach (Command* command, process->commands) {
                bool commandFound = false;
                int i = 0;
                for (i = 0; i < ui->commandList->count(); i++) {
                    if(command->commandName == ui->commandList->item(i)->text())
                    {
                        commandFound = true;
                        break;
                    }
                }

                if(!commandFound)
                    ui->commandList->removeItemWidget(ui->commandList->item(i));

                if(command->commandName == selectedCommandName)
                {
                    selectedCommand = command;
                    isSelectedCommandStillAvailable = true;
                }
            }
        }

        foreach (Command* command, process->commands) {
            bool commandFound = false;
            for (int i = 0; i < ui->commandList->count(); i++) {
                if(command->commandName == ui->commandList->item(i)->text())
                {
                    commandFound = true;
                    break;
                }
            }

            if(!commandFound)
                ui->commandList->addItem(command->commandName);
        }

        ui->commandList->sortItems(Qt::AscendingOrder);

        if(!isSelectedCommandStillAvailable)
        {
            if(isCommandRegistered && this->isVisible())
                stopListeningSlot();

            ui->commandList->setCurrentRow(0);
            selectedCommandChangedSlot();
        }
        else
        {
            commandsAvailable();
        }
    }
    else
    {
        noCommandAvailable();
    }

}

void CommandDialog::sendCommandMessageSlot()
{
    if(ui->commandMessage->toPlainText() != "")
        emit sendCommandMessageSignal(selectedCommandName, ui->commandMessage->toPlainText());
}

void CommandDialog::startListeningSlot()
{
    if(!isCommandRegistered && this->isVisible()) {
        ui->receiverAllCommands->setEnabled(false);
        ui->startListeningButton->setEnabled(false);
        ui->stopListeningButton->setEnabled(true);

        if (ui->receiverAllCommands->isChecked())
        {
            ui->commandOutputLabel->setText("Received messages of all commands:");
            ui->commandOutput->clear();

            foreach (Command* command, process->commands) {
                emit registerCommandSignal(command->commandName);
                isCommandRegistered = true;
                listOfRegisteredCommands.append(command->commandName);
            }
        }
        else
        {
            ui->commandOutputLabel->setText("Received messages of command " + selectedCommandName + ":");
            ui->commandOutput->clear();

            emit registerCommandSignal(selectedCommandName);
            isCommandRegistered = true;
            listOfRegisteredCommands.append(selectedCommandName);
        }
    }
}

void CommandDialog::stopListeningSlot()
{
    if(isCommandRegistered && this->isVisible())
    {
        ui->receiverAllCommands->setEnabled(true);
        ui->stopListeningButton->setEnabled(false);
        ui->startListeningButton->setEnabled(true);

        foreach (QString commandName, listOfRegisteredCommands) {
            emit unRegisterCommandSignal(commandName);
            listOfRegisteredCommands.removeAll(commandName);
        }
        isCommandRegistered = false;
    }
}

void CommandDialog::updateCommandOutput(QString commandName, QByteArray message)
{
    if(this->isVisible())
    {
        if(isCommandRegistered && listOfRegisteredCommands.contains(commandName))
        {
            if(ui->receiverAllCommands->isChecked())
                ui->commandOutput->appendPlainText(commandName + ": " + DIALOGCommon::transformMessageToPlainText(message));
            else
                ui->commandOutput->appendPlainText(DIALOGCommon::transformMessageToPlainText(message));
        }
    }
}

void CommandDialog::receiverAllCommandsSlot()
{
    if(process->commands.size() > 0)
    {
        if(ui->receiverAllCommands->isChecked())
        {
            ui->commandList->setEnabled(false);
            ui->commandMessage->setEnabled(false);
            ui->sendCommandButton->setEnabled(false);
        }
        else
        {
            ui->commandList->setEnabled(true);
            ui->commandMessage->setEnabled(true);
            ui->sendCommandButton->setEnabled(true);

            selectedCommandChangedSlot();
        }
    }
}

void CommandDialog::commandsAvailable()
{
    ui->commandListLabel->setText("Commands:");
    ui->commandMessageLabel->setText("Send message to command " + selectedCommandName + ":");
    ui->commandMessage->setEnabled(true);
    ui->sendCommandButton->setEnabled(true);

    if(!isCommandRegistered)
    {
        ui->receiverAllCommands->setEnabled(true);
        ui->startListeningButton->setEnabled(true);
        ui->stopListeningButton->setEnabled(false);
    }
}

void CommandDialog::noCommandAvailable()
{
    if(isCommandRegistered && this->isVisible())
        stopListeningSlot();

    ui->commandList->clear();

    selectedCommand= NULL;
    selectedCommandName = "";

    ui->commandListLabel->setText("Commands: (no commands available)");
    ui->commandMessageLabel->setText("Command message:");
    ui->commandMessage->clear();
    ui->commandMessage->setEnabled(false);
    ui->sendCommandButton->setEnabled(false);
    ui->receiverAllCommands->setEnabled(false);
    ui->startListeningButton->setEnabled(false);
    ui->stopListeningButton->setEnabled(false);

    ui->commandOutputLabel->setText("Received messages:");
}

void CommandDialog::hideEvent(QHideEvent * event)
{
    if(isCommandRegistered && !this->isVisible())
    {
        foreach (QString commandName, listOfRegisteredCommands) {
            emit unRegisterCommandSignal(commandName);
            listOfRegisteredCommands.removeAll(commandName);
        }
        isCommandRegistered = false;
    }

    selectedCommand = NULL;
    selectedCommandName = "";

    if(process->commands.count() > 0)
    {
        ui->commandOutput->clear();
        ui->commandList->setCurrentRow(0);
    }
}

CommandDialog::~CommandDialog()
{
    delete ui;
}
