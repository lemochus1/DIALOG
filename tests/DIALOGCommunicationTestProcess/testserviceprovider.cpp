#include "testserviceprovider.h"

TESTServicePublisher::TESTServicePublisher(QString nameInit, QString processNameInit, int updatePeriodInit)
    : DIALOGServicePublisher(nameInit),
     updatePeriod(updatePeriodInit),
     processName(processNameInit),
     updateCounter(0)
{

}

void TESTServicePublisher::start()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TESTServicePublisher::updateData);
    timer->start(updatePeriod * 1000);
}

void TESTServicePublisher::updateData()
{
    mutex.lock();
    QByteArray message;
    message.append(QString::number(updateCounter));
    updateCounter++;
    mutex.unlock();
    updateDataSlot(message);
    std::cout << "Send service data update: " << getName().toStdString() << " - " << message.toStdString() << std::endl;
    APIMessageLogger::getInstance().logServiceDataSent(getName(), message);
}
