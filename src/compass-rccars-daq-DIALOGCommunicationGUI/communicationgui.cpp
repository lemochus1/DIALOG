#include "communicationgui.h"
#include "ui_communicationgui.h"

CommunicationGUI::CommunicationGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CommunicationGUI)
{
    ui->setupUi(this);
    this->setFixedSize(1050, 600);

    communicationControlServerNotFound = false;

    ui->nameOfProcessLabel->setGeometry(10, 15, 120, 17);
    ui->nameOfProcessText->setGeometry(130, 10, 120, 27);
    ui->pidLabel->setGeometry(10, 45, 120, 17);
    ui->pidText->setGeometry(130, 40, 120, 27);

    ui->addressLabel->setGeometry(260, 15, 65, 17);
    ui->addressText->setGeometry(325, 10, 120, 27);
    ui->portLabel->setGeometry(260, 45, 65, 17);
    ui->portText->setGeometry(325, 40, 120, 27);

    ui->nameOfProvidedServiceLabel->setGeometry(455, 15, 135, 17);
    ui->nameOfProvidedServiceText->setGeometry(590, 10, 120, 27);
    ui->nameOfSubscribedServiceLabel->setGeometry(455, 45, 135, 17);
    ui->nameOfSubscribedServiceText->setGeometry(590, 40, 120, 27);

    ui->nameOfCommandLabel->setGeometry(720, 15, 80, 17);
    ui->nameOfCommandText->setGeometry(800, 10, 120, 27);

    ui->searchButton->setGeometry(950, 10, 90, 27);
    ui->resetButton->setGeometry(950, 40, 90, 27);

    ui->connectedProcessesTable->move(0, 80);
    ui->connectedProcessesTable->setFixedSize(1050, 483);
    ui->connectedProcessesTable->setColumnWidth(0, 160);
    ui->connectedProcessesTable->setColumnWidth(1, 200);
    ui->connectedProcessesTable->setColumnWidth(2, 80);
    ui->connectedProcessesTable->setColumnWidth(3, 80);
    ui->connectedProcessesTable->setColumnWidth(4, 180);
    ui->connectedProcessesTable->setColumnWidth(5, 190);
    ui->connectedProcessesTable->setColumnWidth(6, 110);
    ui->connectedProcessesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->connectedProcessesTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->connectedProcessesTable->verticalHeader()->setFixedWidth(30);
    ui->connectedProcessesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->connectedProcessesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->connectedProcessesTable->sortByColumn(0, Qt::AscendingOrder);
    ui->connectedProcessesTable->horizontalHeader()->setSortIndicatorShown(true);

    QObject::connect(ui->connectedProcessesTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &CommunicationGUI::sortConnectedProcessesTableSlot);
}

void CommunicationGUI::updateConnectedProcessesTableSlot(GUIInfo *guiInfoInit, GUIInfo* guiInfoOld)
{
    guiInfo = guiInfoInit;
    if(communicationControlServerNotFound)
    {
        clearConnectedProcessesTable();
        communicationControlServerNotFound = false;
        ui->searchButton->setEnabled(true);
    }

    uint numberOfRemoveRows = 0;
    int numberOfRows = ui->connectedProcessesTable->rowCount();
    for(int i = 0; i < numberOfRows; i++)
    {
        bool found = false;
        QString rowKey = ui->connectedProcessesTable->item(i - numberOfRemoveRows,1)->text() + SEPARATOR + ui->connectedProcessesTable->item(i - numberOfRemoveRows,2)->text();
        foreach(Process* process, guiInfo->processes)
        {
            if(rowKey == process->processKey)
            {
                found = true;
                break;
            }
        }

        /* FILTER */

        if(found)
        {
            if(!ui->nameOfProcessText->text().isEmpty() && !guiInfo->processes[rowKey]->processName.toLower().contains(ui->nameOfProcessText->text().toLower()))
                found = false;
            if(!ui->pidText->text().isEmpty() && guiInfo->processes[rowKey]->processPID != ui->pidText->text().toULongLong())
                found = false;
            if(!ui->addressText->text().isEmpty() && !guiInfo->processes[rowKey]->processAddress.toLower().contains(ui->addressText->text().toLower()))
                found = false;
            if(!ui->portText->text().isEmpty() && guiInfo->processes[rowKey]->processPort != ui->portText->text().toUShort())
                found = false;
            if(!ui->nameOfProvidedServiceText->text().isEmpty())
            {
                bool foundService = false;
                foreach(Service* service, guiInfo->processes[rowKey]->servicesAsSender)
                {
                    if(service->serviceName.toLower().contains(ui->nameOfProvidedServiceText->text().toLower()))
                    {
                        foundService = true;
                        break;
                    }
                }
                if(!foundService)
                    found = false;
            }
            if(!ui->nameOfSubscribedServiceText->text().isEmpty())
            {
                bool foundService = false;
                foreach(Service* service, guiInfo->processes[rowKey]->servicesAsReceiver)
                {
                    if(service->serviceName.toLower().contains(ui->nameOfSubscribedServiceText->text().toLower()))
                    {
                        foundService = true;
                        break;
                    }
                }
                if(!foundService)
                    found = false;
            }
            if(!ui->nameOfCommandText->text().isEmpty())
            {
                bool foundCommand = false;
                foreach(Command* command, guiInfo->processes[rowKey]->commands)
                {
                    if(command->commandName.toLower().contains(ui->nameOfCommandText->text().toLower()))
                    {
                        foundCommand = true;
                        break;
                    }
                }
                if(!foundCommand)
                    found = false;
            }
        }

        /* ------ */

        if(!found)
        {
            foreach (QString serviceName, showServiceButtons[rowKey]->serviceDialog->listOfSubscribedServices)
                emit unSubscribeServiceSlot(serviceName);
            showServiceButtons.remove(rowKey);
            delete showServiceButtons[rowKey];

            foreach (QString serviceName, showReceiverButtons[rowKey]->receiverDialog->listOfSubscribedServices)
                emit unSubscribeServiceSlot(serviceName);
            showReceiverButtons.remove(rowKey);
            delete showReceiverButtons[rowKey];

            foreach (QString commandName, showCommandButtons[rowKey]->commandDialog->listOfRegisteredCommands)
                emit unRegisterCommandSlot(commandName);
            showCommandButtons.remove(rowKey);
            delete showCommandButtons[rowKey];

            ui->connectedProcessesTable->removeRow(i - numberOfRemoveRows);
            numberOfRemoveRows++;
        }
        else
        {
            showServiceButtons[rowKey]->updateShowServiceButton(guiInfo->processes[rowKey]);
            showReceiverButtons[rowKey]->updateShowReceiverButton(guiInfo->processes[rowKey]);
            showCommandButtons[rowKey]->updateShowCommandButton(guiInfo->processes[rowKey]);
        }
    }

    uint i = ui->connectedProcessesTable->rowCount();
    foreach(Process* process, guiInfo->processes)
    {
        bool found = false;
        QString key = process->processKey;
        for(int j = 0; j < ui->connectedProcessesTable->rowCount(); j++)
        {
            QString rowKey = ui->connectedProcessesTable->item(j,1)->text() + SEPARATOR + ui->connectedProcessesTable->item(j,2)->text();
            if(rowKey == key)
            {
                found = true;
                break;
            }
        }

        /* FILTER */

        if(!found)
        {
            if(!ui->nameOfProcessText->text().isEmpty() && !process->processName.toLower().contains(ui->nameOfProcessText->text().toLower()))
                found = true;
            if(!ui->pidText->text().isEmpty() && process->processPID != ui->pidText->text().toULongLong())
                found = true;
            if(!ui->addressText->text().isEmpty() && !process->processAddress.toLower().contains(ui->addressText->text().toLower()))
                found = true;
            if(!ui->portText->text().isEmpty() && process->processPort != ui->portText->text().toUShort())
                found = true;
            if(!ui->nameOfProvidedServiceText->text().isEmpty())
            {
                bool foundService = false;
                foreach(Service* service, process->servicesAsSender)
                {
                    if(service->serviceName.toLower().contains(ui->nameOfProvidedServiceText->text().toLower()))
                    {
                        foundService = true;
                        break;
                    }
                }
                if(!foundService)
                    found = true;
            }
            if(!ui->nameOfSubscribedServiceText->text().isEmpty())
            {
                bool foundService = false;
                foreach(Service* service, process->servicesAsReceiver)
                {
                    if(service->serviceName.toLower().contains(ui->nameOfSubscribedServiceText->text().toLower()))
                    {
                        foundService = true;
                        break;
                    }
                }
                if(!foundService)
                    found = true;
            }
            if(!ui->nameOfCommandText->text().isEmpty())
            {
                bool foundCommand = false;
                foreach(Command* command, process->commands)
                {
                    if(command->commandName.toLower().contains(ui->nameOfCommandText->text().toLower()))
                    {
                        foundCommand = true;
                        break;
                    }
                }
                if(!foundCommand)
                    found = true;
            }
        }

        /* ------ */

        if(!found)
        {
            ui->connectedProcessesTable->insertRow(i);
            QTableWidgetItem *item = new QTableWidgetItem(process->processName);
            ui->connectedProcessesTable->setItem(i, 0, item);
            item = new QTableWidgetItem(process->processAddress);
            ui->connectedProcessesTable->setItem(i, 1, item);
            item = new QTableWidgetItem(QString::number(process->processPort));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->connectedProcessesTable->setItem(i, 2, item);
            item = new QTableWidgetItem(QString::number(process->processPID));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->connectedProcessesTable->setItem(i, 3, item);
            showServiceButtons[key] = new ShowServiceButton(process);
            ui->connectedProcessesTable->setCellWidget(i, 4, showServiceButtons[key]);
            QObject::connect(showServiceButtons[key]->serviceDialog, &ServiceDialog::requestServiceSignal, this, &CommunicationGUI::requestServiceSlot);
            QObject::connect(showServiceButtons[key]->serviceDialog, &ServiceDialog::unSubscribeServiceSignal, this, &CommunicationGUI::unSubscribeServiceSlot);
            showReceiverButtons[key] = new ShowReceiverButton(process);
            ui->connectedProcessesTable->setCellWidget(i, 5, showReceiverButtons[key]);
            QObject::connect(showReceiverButtons[key]->receiverDialog, &ReceiverDialog::requestServiceSignal, this, &CommunicationGUI::requestServiceSlot);
            QObject::connect(showReceiverButtons[key]->receiverDialog, &ReceiverDialog::unSubscribeServiceSignal, this, &CommunicationGUI::unSubscribeServiceSlot);
            showCommandButtons[key] = new ShowCommandButton(process);
            ui->connectedProcessesTable->setCellWidget(i, 6, showCommandButtons[key]);
            QObject::connect(showCommandButtons[key]->commandDialog, &CommandDialog::sendCommandMessageSignal, this, &CommunicationGUI::sendCommandMessageSlot);
            QObject::connect(showCommandButtons[key]->commandDialog, &CommandDialog::registerCommandSignal, this, &CommunicationGUI::registerCommandSlot);
            QObject::connect(showCommandButtons[key]->commandDialog, &CommandDialog::unRegisterCommandSignal, this, &CommunicationGUI::unRegisterCommandSlot);
            i++;
        }
    }
    sortConnectedProcessesTableSlot(ui->connectedProcessesTable->horizontalHeader()->sortIndicatorSection());

    if(guiInfoOld != NULL)
        guiInfoOld->clear();
}

void CommunicationGUI::serviceOutputSlot(QString serviceName, QByteArray message)
{
    if(guiInfo->services.contains(serviceName))
    {
        Service* service = guiInfo->services[serviceName];
        Process* sender = service->sender;
        showServiceButtons[sender->processKey]->serviceDialog->updateServiceOutput(serviceName, message);
        foreach(Process* receiverProcess, service->receivers)
            showReceiverButtons[receiverProcess->processKey]->receiverDialog->updateServiceOutput(serviceName, message);
    }
}

void CommunicationGUI::commandOutputSlot(QString commandName, QByteArray message)
{
    if(guiInfo->commands.contains(commandName))
    {
        Command* command = guiInfo->commands[commandName];
        foreach(Process* receiverProcess, command->receivers)
            showCommandButtons[receiverProcess->processKey]->commandDialog->updateCommandOutput(commandName, message);
    }
}

void CommunicationGUI::communicationControlServerNotFoundSlot()
{
    clearConnectedProcessesTable();

    communicationControlServerNotFound = true;

    ui->connectedProcessesTable->insertRow(0);
    QTableWidgetItem *item = new QTableWidgetItem("CommunicationControlServer is not found.");
    item->setTextAlignment(Qt::AlignCenter);
    ui->connectedProcessesTable->setItem(0, 0, item);
    ui->connectedProcessesTable->setSpan(0,0,1,7);

    ui->searchButton->setEnabled(false);
}

void CommunicationGUI::clearConnectedProcessesTable()
{
    foreach (ShowServiceButton* showServiceButton, showServiceButtons)
        delete showServiceButton;
    showServiceButtons.clear();

    foreach (ShowReceiverButton* showReceiverButton, showReceiverButtons)
        delete showReceiverButton;
    showReceiverButtons.clear();

    foreach (ShowCommandButton* showCommandButton, showCommandButtons)
        delete showCommandButton;
    showCommandButtons.clear();

    ui->connectedProcessesTable->clearContents();
    while (ui->connectedProcessesTable->rowCount() > 0)
    {
        ui->connectedProcessesTable->removeRow(0);
    }
}

void CommunicationGUI::sortConnectedProcessesTableSlot(int index)
{
    ui->connectedProcessesTable->sortByColumn(index, ui->connectedProcessesTable->horizontalHeader()->sortIndicatorOrder());
}

void CommunicationGUI::requestServiceSlot(QString serviceName)
{
    if(subscribedServicesCount.contains(serviceName))
    {
        subscribedServicesCount[serviceName]++;
    }
    else
    {
        subscribedServicesCount[serviceName] = 1;
        emit requestServiceSignal(serviceName);
    }
}

void CommunicationGUI::unSubscribeServiceSlot(QString serviceName)
{
    if(subscribedServicesCount.contains(serviceName))
    {
        subscribedServicesCount[serviceName]--;
        if(subscribedServicesCount[serviceName] == 0)
        {
            subscribedServicesCount.remove(serviceName);
            emit unSubscribeServiceSignal(serviceName);
        }
    }
}

void CommunicationGUI::sendCommandMessageSlot(QString commandName, QString message)
{
    emit sendCommandMessageSignal(commandName, message);
}

void CommunicationGUI::registerCommandSlot(QString commandName)
{
    if(registeredCommandsCount.contains(commandName))
    {
        registeredCommandsCount[commandName]++;
    }
    else
    {
        registeredCommandsCount[commandName] = 1;
        emit registerCommandSignal(commandName);
    }
}

void CommunicationGUI::unRegisterCommandSlot(QString commandName)
{
    if(registeredCommandsCount.contains(commandName))
    {
        registeredCommandsCount[commandName]--;
        if(registeredCommandsCount[commandName] == 0)
        {
            registeredCommandsCount.remove(commandName);
            emit unRegisterCommandSignal(commandName);
        }
    }
}

void CommunicationGUI::on_searchButton_clicked()
{
    if(!communicationControlServerNotFound)
        updateConnectedProcessesTableSlot(guiInfo, NULL);
}

void CommunicationGUI::on_resetButton_clicked()
{
    ui->nameOfProcessText->setText("");
    ui->pidText->setText("");
    ui->addressText->setText("");
    ui->portText->setText("");
    ui->nameOfProvidedServiceText->setText("");
    ui->nameOfSubscribedServiceText->setText("");
    ui->nameOfCommandText->setText("");

    if(!communicationControlServerNotFound)
        updateConnectedProcessesTableSlot(guiInfo, NULL);
}

void CommunicationGUI::closeEvent(QCloseEvent *e)
{
    foreach(QString serviceName, subscribedServicesCount.keys())
        emit unSubscribeServiceSignal(serviceName);

    foreach(QString commandName, registeredCommandsCount.keys())
        emit unRegisterCommandSignal(commandName);

    clearConnectedProcessesTable();

    end(0);

    e->ignore();
}

CommunicationGUI::~CommunicationGUI()
{
    delete ui;
}
