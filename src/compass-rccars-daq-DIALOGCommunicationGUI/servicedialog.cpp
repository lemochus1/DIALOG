#include "servicedialog.h"
#include "ui_servicedialog.h"

ServiceDialog::ServiceDialog(Process* processInit, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServiceDialog)
{
    process = processInit;
    ui->setupUi(this);

    selectedServiceName = "";
    selectedService = NULL;
    isServiceSubscribed = false;

    this->setFixedSize(900, 540);
    this->setWindowTitle("Provided services of process " + process->processName + " running on address " + process->processAddress + " on port " + QString::number(process->processPort) + " with PID " + QString::number(process->processPID));

    ui->serviceListLabel->setGeometry(10,10,258,20);
    ui->serviceList->setGeometry(10,30,258,200);
    ui->serviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->receiverTableLabel->setGeometry(278,10,612,20);
    ui->receiverTable->setGeometry(278,30,612,200);
    ui->receiverTable->setColumnWidth(0, 160);
    ui->receiverTable->setColumnWidth(1, 160);
    ui->receiverTable->setColumnWidth(2, 160);
    ui->receiverTable->setColumnWidth(3, 80);
    ui->receiverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->receiverTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->receiverTable->verticalHeader()->setFixedWidth(30);
    ui->receiverTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->receiverTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->receiverTable->sortByColumn(0, Qt::AscendingOrder);
    ui->receiverTable->horizontalHeader()->setSortIndicatorShown(true);

    ui->serviceOutputLabel->setGeometry(10,240,880,20);
    ui->serviceOutput->setGeometry(10,260,880,233);
    ui->serviceOutput->setReadOnly(true);

    ui->startListeningButton->setGeometry(660,503,110,27);
    ui->stopListeningButton->setGeometry(780,503,110,27);
    ui->stopListeningButton->setEnabled(false);

    if (process->servicesAsSender.count() > 0)
    {
        foreach (Service* service, process->servicesAsSender)
            ui->serviceList->addItem(service->serviceName);
        ui->serviceList->sortItems(Qt::AscendingOrder);
        ui->serviceList->setCurrentRow(0);
    }
    else
    {
        noServiceAvailable();
    }

    QObject::connect(ui->receiverTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &ServiceDialog::sortReceiverTableSlot);
    QObject::connect(ui->serviceList, &QListWidget::itemSelectionChanged, this, &ServiceDialog::selectedServiceChangedSlot);
    QObject::connect(ui->startListeningButton, &QPushButton::clicked, this, &ServiceDialog::startListeningSlot);
    QObject::connect(ui->stopListeningButton, &QPushButton::clicked, this, &ServiceDialog::stopListeningSlot);
}

void ServiceDialog::sortReceiverTableSlot(int index)
{
    ui->receiverTable->sortByColumn(index, ui->receiverTable->horizontalHeader()->sortIndicatorOrder());
}

void ServiceDialog::selectedServiceChangedSlot()
{
    clearReceiverTable();

    if (process->servicesAsSender.count() > 0)
    {
        selectedServiceName = ui->serviceList->selectedItems().first()->text();
        foreach (Service* service, process->servicesAsSender) {
            if(service->serviceName == selectedServiceName)
            {
                selectedService = service;
                break;
            }
        }
        uint i = 0;
        foreach(Process* receiver, selectedService->receivers)
        {
            ui->receiverTable->insertRow(i);
            QTableWidgetItem *item = new QTableWidgetItem(receiver->processName);
            ui->receiverTable->setItem(i, 0, item);
            item = new QTableWidgetItem(receiver->processAddress);
            ui->receiverTable->setItem(i, 1, item);
            item = new QTableWidgetItem(QString::number(receiver->processPort));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->receiverTable->setItem(i, 2, item);
            item = new QTableWidgetItem(QString::number(receiver->processPID));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->receiverTable->setItem(i, 3, item);
            i++;
        }
        sortReceiverTableSlot(ui->receiverTable->horizontalHeader()->sortIndicatorSection());

        servicesAvailable();
    }
    else
    {
        noServiceAvailable();
    }
}

void ServiceDialog::updateServiceDialog(Process *processInit)
{
    process = processInit;
    if (process->servicesAsSender.count() > 0)
    {
        bool isSelectedServiceStillAvailable = false;
        if(ui->serviceList->count() > 0)
        {
            foreach (Service* service, process->servicesAsSender) {
                bool serviceFound = false;
                int i = 0;
                for (i = 0; i < ui->serviceList->count(); i++) {
                    if(service->serviceName == ui->serviceList->item(i)->text())
                    {
                        serviceFound = true;
                        break;
                    }
                }

                if(!serviceFound)
                    ui->serviceList->removeItemWidget(ui->serviceList->item(i));

                if(service->serviceName == selectedServiceName)
                {
                    selectedService = service;
                    isSelectedServiceStillAvailable = true;
                }
            }
        }

        foreach (Service* service, process->servicesAsSender) {
            bool serviceFound = false;
            for (int i = 0; i < ui->serviceList->count(); i++) {
                if(service->serviceName == ui->serviceList->item(i)->text())
                {
                    serviceFound = true;
                    break;
                }
            }

            if(!serviceFound)
                ui->serviceList->addItem(service->serviceName);
        }

        ui->serviceList->sortItems(Qt::AscendingOrder);

        if(!isSelectedServiceStillAvailable)
        {
            if(isServiceSubscribed && this->isVisible())
                stopListeningSlot();

            ui->serviceList->setCurrentRow(0);
            selectedServiceChangedSlot();
        }
        else
        {
            uint numberOfRemoveRows = 0;
            int numberOfRows = ui->receiverTable->rowCount();
            for(int i = 0; i < numberOfRows; i++)
            {
                bool found = false;
                QString rowKey = ui->receiverTable->item(i - numberOfRemoveRows,1)->text() + SEPARATOR + ui->receiverTable->item(i - numberOfRemoveRows,2)->text();
                foreach(Process* receiverProcess, selectedService->receivers)
                {
                    if(rowKey == receiverProcess->processKey)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    ui->receiverTable->removeRow(i - numberOfRemoveRows);
                    numberOfRemoveRows++;
                }
            }

            uint i = ui->receiverTable->rowCount();
            foreach(Process* receiverProcess, selectedService->receivers)
            {
                bool found = false;
                for(int j = 0; j < ui->receiverTable->rowCount(); j++)
                {
                    QString rowKey = ui->receiverTable->item(j,1)->text() + SEPARATOR + ui->receiverTable->item(j,2)->text();
                    if(rowKey == receiverProcess->processKey)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    ui->receiverTable->insertRow(i);
                    QTableWidgetItem *item = new QTableWidgetItem(receiverProcess->processName);
                    ui->receiverTable->setItem(i, 0, item);
                    item = new QTableWidgetItem(receiverProcess->processAddress);
                    ui->receiverTable->setItem(i, 1, item);
                    item = new QTableWidgetItem(QString::number(receiverProcess->processPort));
                    ui->receiverTable->setItem(i, 2, item);
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    item = new QTableWidgetItem(QString::number(receiverProcess->processPID));
                    ui->receiverTable->setItem(i, 3, item);
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    i++;
                }
            }
            sortReceiverTableSlot(ui->receiverTable->horizontalHeader()->sortIndicatorSection());

            servicesAvailable();
        }
    }
    else
    {
        noServiceAvailable();
    }

}

void ServiceDialog::startListeningSlot()
{
    if(!isServiceSubscribed && this->isVisible()) {
        ui->serviceOutputLabel->setText("Output of service " + selectedServiceName + ":");
        ui->serviceOutput->clear();

        ui->startListeningButton->setEnabled(false);
        ui->stopListeningButton->setEnabled(true);

        emit requestServiceSignal(selectedServiceName);
        isServiceSubscribed = true;
        listOfSubscribedServices.append(selectedServiceName);
    }
}

void ServiceDialog::stopListeningSlot()
{
    if(isServiceSubscribed && this->isVisible())
    {
        ui->stopListeningButton->setEnabled(false);
        ui->startListeningButton->setEnabled(true);

        foreach (QString serviceName, listOfSubscribedServices) {
            emit unSubscribeServiceSignal(serviceName);
            listOfSubscribedServices.removeAll(serviceName);
        }
        isServiceSubscribed = false;
    }
}

void ServiceDialog::updateServiceOutput(QString serviceName, QByteArray message)
{
    if(isServiceSubscribed && this->isVisible())
    {
        if(listOfSubscribedServices.contains(serviceName))
            ui->serviceOutput->appendPlainText(DIALOGCommon::transformMessageToPlainText(message));
    }
}

void ServiceDialog::servicesAvailable()
{
    ui->serviceListLabel->setText("Provided services:");
    ui->receiverTableLabel->setText("Receivers of service " + selectedServiceName + ":");

    if(!isServiceSubscribed)
    {
        ui->startListeningButton->setEnabled(true);
        ui->stopListeningButton->setEnabled(false);
    }
}

void ServiceDialog::noServiceAvailable()
{
    if(isServiceSubscribed && this->isVisible())
        stopListeningSlot();

    ui->serviceList->clear();
    clearReceiverTable();

    selectedService = NULL;
    selectedServiceName = "";
    ui->serviceListLabel->setText("Provided services: (no services available)");
    ui->receiverTableLabel->setText("Receivers:");
    ui->serviceOutputLabel->setText("Output:");
    ui->startListeningButton->setEnabled(false);
    ui->stopListeningButton->setEnabled(false);
}

void ServiceDialog::clearReceiverTable()
{
    ui->receiverTable->clearContents();
    while (ui->receiverTable->rowCount() > 0)
    {
        ui->receiverTable->removeRow(0);
    }
}

void ServiceDialog::hideEvent(QHideEvent * event)
{
    if(isServiceSubscribed && !this->isVisible())
    {
        foreach (QString serviceName, listOfSubscribedServices) {
            emit unSubscribeServiceSignal(serviceName);
            listOfSubscribedServices.removeAll(serviceName);
        }
        isServiceSubscribed = false;
    }

    selectedService = NULL;
    selectedServiceName = "";
    if(process->servicesAsSender.count() > 0)
    {
        ui->serviceOutput->clear();
        ui->serviceList->setCurrentRow(0);
    }
}

ServiceDialog::~ServiceDialog()
{
    delete ui;
}
