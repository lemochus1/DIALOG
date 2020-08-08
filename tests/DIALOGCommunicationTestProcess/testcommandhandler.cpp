#include "testcommandhandler.h"
#include <iostream>

TESTCommandHandler::TESTCommandHandler(QString name)
    : DIALOGCommand(name)
{

}

void TESTCommandHandler::commandReceivedSlot(QByteArray message)
{
    std::cout << "Received" << QString(message).toStdString() <<std::endl;
//    qDebug() << "Recieved:" << QString(message);
    APIMessageLogger::getInstance().logCommandReceived(getName(), message);
}
