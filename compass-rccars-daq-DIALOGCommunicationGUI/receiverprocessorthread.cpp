#include "receiverprocessorthread.h"

void ReceiverProcessorThread::messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray* header, QByteArray* message)
{
    QList<QByteArray> headerList = header->split(SEPARATOR);
    if(headerList[0] == INFO_MONITORING)
    {
        GUIInfo* guiInfoOld = guiInfo;
        guiInfo = new GUIInfo();
        guiInfo->readGUIInfo(*message);
        emit updateConnectedProcessesTableSignal(guiInfo, guiInfoOld);
    }
    else if(headerList[0] == SERVICE_MESSAGE)
    {
        if(server->isServiceKnown(headerList[1]))
            emit serviceOutputSignal(headerList[1], *message);
    }
    else if(headerList[0] == COMMAND_MESSAGE)
    {
        if(server->isCommandKnown(headerList[1]))
            emit commandOutputSignal(headerList[1], *message);
    }
    else
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Received message from (" << senderName << ", " << senderPort << "): " << *header << *message << ".";
    }

    delete header;
    delete message;
}

void ReceiverProcessorThread::serverErrorSlot(QString error)
{
    QStringList errorList = error.split(SEPARATOR);

    if(errorList[1] == HOST_NOT_FOUND_ERROR)
    {
        if(errorList[2] == server->getControlServer()->processAddress && errorList[3].toUShort() == server->getControlServer()->processPort)
        {
            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "CommunicationControlServer is not found.";
            communicationControlServerNotFoundSignal();
        }
    }
    else if(errorList[1] == CONNECTION_REFUSED_ERROR)
    {
        if(errorList[2] == server->getControlServer()->processAddress && errorList[3].toUShort() == server->getControlServer()->processPort)
        {
            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "CommunicationControlServer is not found.";
            communicationControlServerNotFoundSignal();
        }
    }
    else
    {
    }
}
void ReceiverProcessorThread::run()
{
    guiInfo = NULL;

    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Receiver EventLoop";
}

ReceiverProcessorThread::~ReceiverProcessorThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Receiver destructor";

    delete guiInfo;
}
