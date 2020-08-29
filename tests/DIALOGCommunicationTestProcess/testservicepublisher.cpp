#include "testservicepublisher.h"

TESTServicePublisher::TESTServicePublisher(QString nameInit,
                                           QString processNameInit,
                                           int updatePeriodInit,
                                           int repeatInit,
                                           int messageSizeInit)
    : DIALOGServicePublisher(nameInit),
      processName(processNameInit),
      updatePeriod(updatePeriodInit),
      updateCounter(0),
      repeat(repeatInit),
      size(messageSizeInit)
{
    connect(this, &DIALOGServicePublisher::subscriberLostSignal,
            this, &TESTServicePublisher::lostSubscriber);
    connect(this, &DIALOGServicePublisher::subscriberConnectedSignal,
            this, &TESTServicePublisher::newSubscriber);
}

void TESTServicePublisher::start()
{
    timer = new QTimer(this);
    timer->singleShot(updatePeriod, this, &TESTServicePublisher::updateData);
}

void TESTServicePublisher::updateData()
{
    if (hasSubscriber()) {
        QByteArray message;
        message.append(QString::number(updateCounter));
        if (size > 0) {
            message.append("-" + QString::number(size) + "-");
            message.append(APIMessageLogger::GetInstance().generateRandomString(size));
        }
        updateDataSlot(message);

        QString asString = APIMessageLogger::GetInstance().getMessageLogString(message);

        std::cout << "Send service data update: " << getName().toStdString()
                  << " - " << asString.toStdString() << std::endl;
        APIMessageLogger::GetInstance().logServiceDataSent(getName(), asString);

    } else {
        std::cout << "No need to update data as no subscriber is connected." << std::endl;
    }
    updateCounter++;
    if (updateCounter < repeat) {
        timer->singleShot(updatePeriod, this, &TESTServicePublisher::updateData);
    }
}

void TESTServicePublisher::dataUpdateRequestedSlot()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Data update requested." << std::endl;
}

void TESTServicePublisher::controlServerConnectedSlot()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Control server connected." << std::endl;
}

void TESTServicePublisher::controlServerUnavailableErrorSlot()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Control server connected." << std::endl;
}

void TESTServicePublisher::messageReceivedSlot(const QByteArray &message)
{
    std::cout << "Service publisher: " << getName().toStdString() << ": Message received "
              << QString(message).toStdString()<< std::endl;
}

void TESTServicePublisher::registrationFailedSlot()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Registration failed."<< std::endl;
}

void TESTServicePublisher::newSubscriber()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Subscriber connected." << std::endl;
}

void TESTServicePublisher::lostSubscriber()
{
    std::cout << "Service publisher: " << getName().toStdString()
              << ": Subscriber lost." << std::endl;
}
