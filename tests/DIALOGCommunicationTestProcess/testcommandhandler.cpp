#include "testcommandhandler.h"
#include <iostream>

TESTCommandHandler::TESTCommandHandler(QString name)
    : DIALOGCommand(name)
{

}

void TESTCommandHandler::commandReceivedSlot(QByteArray message)
{
    QString asString = APIMessageLogger::getInstance().getMessageLogString(message);
    std::cout << "Received command: " << getName().toStdString() <<" - "<< asString.toStdString() << std::endl;
    APIMessageLogger::getInstance().logCommandReceived(getName(), asString);
}
