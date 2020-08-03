#include "receiverdialog.h"
#include "ui_receiverdialog.h"

ReceiverDialog::ReceiverDialog(Process* processInit, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceiverDialog)
{
    process = processInit;
    ui->setupUi(this);

    selectedServiceName = "";
    selectedService = NULL;
    isServiceSubscribed = false;

    this->setFixedSize(900, 540);
    this->setWindowTitle("Subscribed services of process " + process->processName + " running on address " + process->processAddress + " on port " + QString::number(process->processPort) + " with PID " + QString::number(process->processPID));

    ui->serviceListLabel->setGeometry(10,10,298,20);
    ui->serviceList->setGeometry(10,30,298,200);
    ui->serviceList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->senderTableLabel->setGeometry(318,10,572,20);
    ui->senderTable->setGeometry(318,30,572,200);
    ui->senderTable->setColumnWidth(0, 160);
    ui->senderTable->setColumnWidth(1, 200);
    ui->senderTable->setColumnWidth(2, 80);
    ui->senderTable->setColumnWidth(3, 80);
    ui->senderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->senderTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->senderTable->verticalHeader()->setFixedWidth(30);
    ui->senderTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->senderTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->senderTable->sortByColumn(0, Qt::AscendingOrder);
    ui->senderTable->horizontalHeader()->setSortIndicatorShown(true);

    ui->serviceOutputLabel->setGeometry(10,240,620,20);
    ui->serviceOutput->setGeometry(10,260,880,233);
    ui->serviceOutput->setReadOnly(true);

    ui->receiverAllServices->setGeometry(630,240,260,20);

    ui->startListeningButton->setGeometry(660,503,110,27);
    ui->stopListeningButton->setGeometry(780,503,110,27);
    ui->stopListeningButton->setEnabled(false);

    if (process->servicesAsReceiver.count() > 0)
    {
        foreach (Service* service, process->servicesAsReceiver)
            ui->serviceList->addItem(service->serviceName);
        ui->serviceList->sortItems(Qt::AscendingOrder);
        ui->serviceList->setCurrentRow(0);
    }
    else
    {
        noServiceAvailable();
    }

    QObject::connect(ui->senderTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &ReceiverDialog::sortSenderTableSlot);
    QObject::connect(ui->serviceList, &QListWidget::itemSelectionChanged, this, &ReceiverDialog::selectedServiceChangedSlot);
    QObject::connect(ui->receiverAllServices, &QCheckBox::clicked, this, &ReceiverDialog::receiverAllServicesSlot);
    QObject::connect(ui->startListeningButton, &QPushButton::clicked, this, &ReceiverDialog::startListeningSlot);
    QObject::connect(ui->stopListeningButton, &QPushButton::clicked, this, &ReceiverDialog::stopListeningSlot);

}

void ReceiverDialog::sortSenderTableSlot(int index)
{
    ui->senderTable->sortByColumn(index, ui->senderTable->horizontalHeader()->sortIndicatorOrder());
}

void ReceiverDialog::selectedServiceChangedSlot()
{
    clearSenderTable();

    if (process->servicesAsReceiver.count() > 0)
    {
        selectedServiceName = ui->serviceList->selectedItems().first()->text();

        foreach (Service* service, process->servicesAsReceiver) {
            if(service->serviceName == selectedServiceName)
            {
                selectedService = service;
                break;
            }
        }
        if(selectedService->sender != NULL)
        {
            ui->senderTable->insertRow(0);
            QTableWidgetItem *item = new QTableWidgetItem(selectedService->sender->processName);
            ui->senderTable->setItem(0, 0, item);
            item = new QTableWidgetItem(selectedService->sender->processAddress);
            ui->senderTable->setItem(0, 1, item);
            item = new QTableWidgetItem(QString::number(selectedService->sender->processPort));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->senderTable->setItem(0, 2, item);
            item = new QTableWidgetItem(QString::number(selectedService->sender->processPID));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->senderTable->setItem(0, 3, item);
        }
        sortSenderTableSlot(ui->senderTable->horizontalHeader()->sortIndicatorSection());

        servicesAvailable();
    }
    else
    {
        noServiceAvailable();
    }
}

void ReceiverDialog::updateReceiverDialog(Process *processInit)
{
    process = processInit;

    if (process->servicesAsReceiver.count() > 0)
    {
        bool isSelectedServiceStillAvailable = false;
        if(ui->serviceList->count() > 0)
        {
            foreach (Service* service, process->servicesAsReceiver) {
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

        foreach (Service* service, process->servicesAsReceiver) {
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
            if(selectedService->sender == NULL)
            {
                clearSenderTable();
            }
            else
            {
                bool found = false;
                if (ui->senderTable->rowCount() > 0)
                {
                    QString rowKey = ui->senderTable->item(0, 1)->text() + SEPARATOR + ui->senderTable->item(0, 2)->text();
                    if(rowKey == selectedService->sender->processKey)
                        found = true;
                }

                if(!found)
                {
                    clearSenderTable();
                    ui->senderTable->insertRow(0);
                    QTableWidgetItem *item = new QTableWidgetItem(selectedService->sender->processName);
                    ui->senderTable->setItem(0, 0, item);
                    item = new QTableWidgetItem(selectedService->sender->processAddress);
                    ui->senderTable->setItem(0, 1, item);
                    item = new QTableWidgetItem(QString::number(selectedService->sender->processPort));
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    ui->senderTable->setItem(0, 2, item);
                    item = new QTableWidgetItem(QString::number(selectedService->sender->processPID));
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    ui->senderTable->setItem(0, 3, item);
                }

                sortSenderTableSlot(ui->senderTable->horizontalHeader()->sortIndicatorSection());

                servicesAvailable();
            }
        }
    }
    else
    {
        noServiceAvailable();
    }

}

void ReceiverDialog::startListeningSlot()
{
    if(!isServiceSubscribed && this->isVisible()) {
        ui->receiverAllServices->setEnabled(false);
        ui->startListeningButton->setEnabled(false);
        ui->stopListeningButton->setEnabled(true);

        if (ui->receiverAllServices->isChecked())
        {
            ui->serviceOutputLabel->setText("Output of all subscribed services:");
            ui->serviceOutput->clear();

            foreach (Service* service, process->servicesAsReceiver) {
                emit requestServiceSignal(service->serviceName);
                isServiceSubscribed = true;
                listOfSubscribedServices.append(service->serviceName);
            }
        }
        else
        {
            ui->serviceOutputLabel->setText("Output of service " + selectedServiceName + ":");
            ui->serviceOutput->clear();

            emit requestServiceSignal(selectedServiceName);
            isServiceSubscribed = true;
            listOfSubscribedServices.append(selectedServiceName);
        }
    }
}

void ReceiverDialog::stopListeningSlot()
{
    if(isServiceSubscribed && this->isVisible())
    {
        ui->receiverAllServices->setEnabled(true);
        ui->stopListeningButton->setEnabled(false);
        ui->startListeningButton->setEnabled(true);

        foreach (QString serviceName, listOfSubscribedServices) {
            emit unSubscribeServiceSignal(serviceName);
            listOfSubscribedServices.removeAll(serviceName);
        }
        isServiceSubscribed = false;
    }
}


void ReceiverDialog::updateServiceOutput(QString serviceName, QByteArray message)
{
    if(this->isVisible())
    {
        if(isServiceSubscribed && listOfSubscribedServices.contains(serviceName))
        {
            if(ui->receiverAllServices->isChecked())
                ui->serviceOutput->appendPlainText(serviceName + ": " + DIALOGCommon::transformMessageToPlainText(message));
            else
                ui->serviceOutput->appendPlainText(DIALOGCommon::transformMessageToPlainText(message));
        }
    }
}

void ReceiverDialog::receiverAllServicesSlot()
{
    if(process->servicesAsReceiver.size() > 0)
    {
        if (ui->receiverAllServices->isChecked())
        {
            ui->serviceList->setEnabled(false);
            ui->senderTable->setEnabled(false);
        }
        else
        {
            ui->serviceList->setEnabled(true);
            ui->senderTable->setEnabled(true);

            selectedServiceChangedSlot();
        }
    }
}

void ReceiverDialog::servicesAvailable()
{
    ui->serviceListLabel->setText("Subscribed services:");
    ui->senderTableLabel->setText("Sender of service " + selectedServiceName + ":");

    if(!isServiceSubscribed)
    {
        ui->receiverAllServices->setEnabled(true);
        ui->startListeningButton->setEnabled(true);
        ui->stopListeningButton->setEnabled(false);
    }
}

void ReceiverDialog::noServiceAvailable()
{
    if(isServiceSubscribed && this->isVisible())
        stopListeningSlot();

    ui->serviceList->clear();
    clearSenderTable();

    selectedService = NULL;
    selectedServiceName = "";

    ui->serviceListLabel->setText("Subscribed services: (no services available)");
    ui->senderTableLabel->setText("Sender:");
    ui->serviceOutputLabel->setText("Output:");
    ui->receiverAllServices->setEnabled(false);
    ui->startListeningButton->setEnabled(false);
    ui->stopListeningButton->setEnabled(false);
}

void ReceiverDialog::clearSenderTable()
{
    ui->senderTable->clearContents();
    while (ui->senderTable->rowCount() > 0)
    {
        ui->senderTable->removeRow(0);
    }
}

void ReceiverDialog::hideEvent(QHideEvent * event)
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

    if(process->servicesAsReceiver.count() > 0)
    {
        ui->serviceOutput->clear();
        ui->serviceList->setCurrentRow(0);
    }
}

ReceiverDialog::~ReceiverDialog()
{
    delete ui;
}
