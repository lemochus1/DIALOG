#include "testserviceprovider.h"

TESTServicePublisher::TESTServicePublisher(QString nameInit, QString processNameInit, int updatePeriodInit, int repeatInit, int messageSizeInit)
    : DIALOGServicePublisher(nameInit),
     updatePeriod(updatePeriodInit),
     processName(processNameInit),
     updateCounter(0),
     repeat(repeatInit),
     size(messageSizeInit)
{

}

void TESTServicePublisher::start()
{
    timer = new QTimer(this);
    timer->singleShot(updatePeriod, this, &TESTServicePublisher::updateData);
}

void TESTServicePublisher::updateData()
{
    mutex.lock();
    QByteArray message;
    message.append(QString::number(updateCounter));
    if (size > 0) {
        message.append("-" + QString::number(size) + "-");
        message.append(APIMessageLogger::getInstance().generateRandomString(size));
    }
    updateCounter++;
    mutex.unlock();
    QString asString = APIMessageLogger::getInstance().getMessageLogString(message);

    updateDataSlot(message);
    std::cout << "Send service data update: " << getName().toStdString() << " - " << asString.toStdString() << std::endl;

    APIMessageLogger::getInstance().logServiceDataSent(getName(), asString);

    if (updateCounter < repeat) {
        timer->singleShot(updatePeriod, this, &TESTServicePublisher::updateData);
    }
}
