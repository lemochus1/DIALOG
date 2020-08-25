#include "testcommandhandler.h"
#include <iostream>

TESTCommandHandler::TESTCommandHandler(QString name)
    : DIALOGCommandHandler(name)
{

}

void TESTCommandHandler::commandReceivedSlot(QByteArray message)
{
    QString asString = APIMessageLogger::GetInstance().getMessageLogString(message);
    std::cout << "Received command: " << getName().toStdString() <<" - "<< asString.toStdString() << std::endl;
    APIMessageLogger::GetInstance().logCommandReceived(getName(), asString);
}
