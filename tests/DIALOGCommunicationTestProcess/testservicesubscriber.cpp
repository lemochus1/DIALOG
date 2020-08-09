#include "testservicesubscriber.h"

TESTServiceSubscriber::TESTServiceSubscriber(QString name)
    : DIALOGServiceSubscriber(name)
{

}

void TESTServiceSubscriber::dataUpdatedSlot(QByteArray dataInit)
{
    data = dataInit;
    APIMessageLogger::getInstance().logServiceDataReceived(getName(), dataInit);
    std::cout << "Service data received: " << getName().toStdString() << " - " << dataInit.toStdString() << std::endl;
}
