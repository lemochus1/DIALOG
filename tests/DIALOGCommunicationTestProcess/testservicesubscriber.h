#ifndef TESTSERVICESUBSCRIBER_H
#define TESTSERVICESUBSCRIBER_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTServiceSubscriber : public DIALOGServiceSubscriber
{
    Q_OBJECT

public:
    explicit TESTServiceSubscriber(QString name);

private slots:
    virtual void dataUpdatedSlot(const QByteArray &data) override;
    virtual void controlServerConnectedSlot() override;
    virtual void controlServerUnavailableErrorSlot() override;
    virtual void messageReceivedSlot(const QByteArray& message) override;
    virtual void serviceUnavailableErrorSlot() override;
    virtual void serviceActivatedSlot() override;

};

#endif // TESTSERVICESUBSCRIBER_H
