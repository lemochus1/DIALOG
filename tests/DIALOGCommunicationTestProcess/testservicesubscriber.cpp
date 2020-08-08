#include "testservicesubscriber.h"

TESTServiceSubscriber::TESTServiceSubscriber(QString name)
    : DIALOGServiceSubscriber(name)
{

}

void TESTServiceSubscriber::dataUpdatedSlot(QByteArray dataInit)
{
    data = dataInit;
    APIMessageLogger::getInstance().logServiceDataReceived(getName(), dataInit);
}
