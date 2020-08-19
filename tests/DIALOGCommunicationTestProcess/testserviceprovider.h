#ifndef TESTSERVICEPROVIDER_H
#define TESTSERVICEPROVIDER_H

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

signals:

private:
    QString processName;
    int updatePeriod;
    int updateCounter;
    int repeat;
    int size;
    QMutex mutex;
    QTimer* timer;

};

#endif // TESTSERVICEPROVIDER_H
