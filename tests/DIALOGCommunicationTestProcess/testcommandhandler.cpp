#include "testcommandhandler.h"
#include <iostream>

TESTCommandHandler::TESTCommandHandler(const QString& name)
    : DIALOGCommandHandler(name)
{}

void TESTCommandHandler::commandReceivedSlot(const QByteArray &message)
{
    QString asString = APIMessageLogger::GetInstance().getMessageLogString(message);

    std::cout << "Received command: " << getName().toStdString()
              <<" - "<< asString.toStdString() << std::endl;

    APIMessageLogger::GetInstance().logCommandReceived(getName(), asString);
}

void TESTCommandHandler::controlServerConnectedSlot()
{
    std::cout << "Command handler: " << getName().toStdString()
              << ": Control server connected." << std::endl;
}

void TESTCommandHandler::controlServerUnavailableErrorSlot()
{
    std::cout << "Command handler: " << getName().toStdString()
              << ": Control server lost." << std::endl;
    APIMessageLogger::GetInstance().logControlServerLost("command", getName());
}
