#ifndef SERVICEDIALOG_H
#define SERVICEDIALOG_H

#include <QWidget>
#include "server.h"

namespace Ui {
class ServiceDialog;
}

class ServiceDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceDialog(Process* processInit, QWidget *parent = 0);
    ~ServiceDialog();
    void updateServiceDialog(Process* processInit);
    void updateServiceOutput(QString serviceName, QByteArray message);
    void servicesAvailable();
    void noServiceAvailable();
    void clearReceiverTable();
    void hideEvent(QHideEvent * event);
    Process* process;
    Service* selectedService;
    QString selectedServiceName;
    bool isServiceSubscribed;
    QList<QString> listOfSubscribedServices;

public slots:
    void sortReceiverTableSlot(int index);
    void selectedServiceChangedSlot();
    void startListeningSlot();
    void stopListeningSlot();

signals:
    void requestServiceSignal(QString serviceName);
    void unSubscribeServiceSignal(QString serviceName);

private:
    Ui::ServiceDialog *ui;
};

#endif // SERVICEDIALOG_H
