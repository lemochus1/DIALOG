#include "testservicesubscriber.h"

TESTServiceSubscriber::TESTServiceSubscriber(QString name)
    : DIALOGServiceSubscriber(name)
{

}

void TESTServiceSubscriber::dataUpdatedSlot(const QByteArray& data)
{
    QString asString = APIMessageLogger::GetInstance().getMessageLogString(data);
    std::cout << "Service data received: " << getName().toStdString()
              << " - " << asString.toStdString() << std::endl;
    APIMessageLogger::GetInstance().logServiceDataReceived(getName(), asString);
}

void TESTServiceSubscriber::controlServerConnectedSlot()
{
    std::cout << "Service subscriber: " << getName().toStdString()
              << ": Control server connected." << std::endl;
}

void TESTServiceSubscriber::controlServerUnavailableErrorSlot()
{
    std::cout << "Service subscriber: " << getName().toStdString()
              << ": Control server lost."  << std::endl;
    APIMessageLogger::GetInstance().logControlServerLost("command", getName());
}

void TESTServiceSubscriber::messageReceivedSlot(const QByteArray &message)
{
    std::cout << "Service subscriber: " << getName().toStdString() << ": Message received: "
              << QString(message).toStdString() << std::endl;;
}

void TESTServiceSubscriber::serviceUnavailableErrorSlot()
{
    APIMessageLogger::GetInstance().logServiceUnavailable(getName());
    std::cout << "Service subscriber: " << getName().toStdString()
              << ": Service is unavailable." << std::endl;
}

void TESTServiceSubscriber::serviceActivatedSlot()
{
    std::cout << "Service subscriber: " << getName().toStdString()
              << ": Service was activated." << std::endl;
}
