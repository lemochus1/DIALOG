#include "testcommandhandler.h"
#include <iostream>

TESTCommandHandler::TESTCommandHandler(QString name)
    : DIALOGCommand(name)
{

}

void TESTCommandHandler::commandReceivedSlot(QByteArray message)
{
    std::cout << "Received command: " << getName().toStdString() <<" - "<< QString(message).toStdString() << std::endl;
    APIMessageLogger::getInstance().logCommandReceived(getName(), message);
}
