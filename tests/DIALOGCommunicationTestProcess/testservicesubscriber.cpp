#include "testservicesubscriber.h"

TESTServiceSubscriber::TESTServiceSubscriber(QString name)
    : DIALOGServiceSubscriber(name)
{

}

void TESTServiceSubscriber::dataUpdatedSlot(QByteArray dataInit)
{
    data = dataInit;
    QString asString = APIMessageLogger::getInstance().getMessageLogString(dataInit);
    std::cout << "Service data received: " << getName().toStdString() <<" - "<< asString.toStdString() << std::endl;
    APIMessageLogger::getInstance().logServiceDataReceived(getName(), asString);
}
