#ifndef RECEIVERDIALOG_H
#define RECEIVERDIALOG_H

#include <QWidget>
#include "server.h"

namespace Ui {
class ReceiverDialog;
}

class ReceiverDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiverDialog(Process* processInit, QWidget *parent = 0);
    ~ReceiverDialog();
    void updateReceiverDialog(Process* processInit);
    void updateServiceOutput(QString serviceName, QByteArray message);
    void servicesAvailable();
    void noServiceAvailable();
    void clearSenderTable();
    void hideEvent(QHideEvent * event);
    Process* process;
    Service* selectedService;
    QString selectedServiceName;
    bool isServiceSubscribed;
    QList<QString> listOfSubscribedServices;

public slots:
    void sortSenderTableSlot(int index);
    void selectedServiceChangedSlot();
    void receiverAllServicesSlot();
    void startListeningSlot();
    void stopListeningSlot();

signals:
    void requestServiceSignal(QString serviceName);
    void unSubscribeServiceSignal(QString serviceName);

private:
    Ui::ReceiverDialog *ui;
};

#endif // RECEIVERDIALOG_H
