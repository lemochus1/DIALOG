#ifndef TESTServicePublisher_H
#define TESTServicePublisher_H

#include <QObject>
#include "dialogapi.h"
#include "apimessagelogger.h"

class TESTServicePublisher : public DIALOGServicePublisher
{
    Q_OBJECT

public:
    TESTServicePublisher(QString nameInit, QString processNameInit, int updatePeriodInit=1,
                         int repeatInit = 100, int messageSizeInit = 0);

    void start();

public slots:
    void updateData();

    virtual void dataUpdateRequestedSlot() override;
    virtual void controlServerConnectedSlot() override;
    virtual void controlServerUnavailableErrorSlot() override;
    virtual void messageReceivedSlot(const QByteArray &message) override;
    virtual void registrationFailedSlot() override;

    void newSubscriber();
    void lostSubscriber();

private:
    QString processName;
    int updatePeriod;
    int updateCounter;
    int repeat;
    int size;
    QTimer* timer;

};

#endif // TESTServicePublisher_H
