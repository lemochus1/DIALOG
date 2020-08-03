#include "receiverprocessorthread.h"

void ReceiverProcessorThread::messageReceivedSlot(QString senderAddress, quint16 senderPort, QByteArray* header, QByteArray* message)
{
    QList<QByteArray> headerList = header->split(SEPARATOR);
    if(headerList[0] == INFO_MONITORING)
    {
        DIALOGInfo* dialogInfoOld = dialogInfo;
        dialogInfo = new DIALOGInfo();
        dialogInfo->readDIALOGInfo(*message);

        emit updateDIALOGInfoSignal(dialogInfo, dialogInfoOld);
    }
    else if(headerList[0] == SERVICE_MESSAGE)
    {
        if(server->isServiceKnown(headerList[1]))
            Q_EMIT serviceOutputSignal(senderAddress, senderPort, *header, *message);
    }
    else if(headerList[0] == COMMAND_MESSAGE)
    {
        if(server->isCommandKnown(headerList[1]))
            Q_EMIT commandOutputSignal(senderAddress, senderPort, *header, *message);
    }
    else
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Received message from (" << senderAddress << ", " << senderPort << "): " << *header << *message << ".";
    }

    delete header;
    delete message;
}

void ReceiverProcessorThread::run()
{
    dialogInfo = NULL;

    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Receiver EventLoop";
}

ReceiverProcessorThread::~ReceiverProcessorThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Receiver destructor";

    delete dialogInfo;
}
