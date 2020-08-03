#include "senderprocessorthread.h"

void SenderProcessorThread::requestServiceSlot(QString serviceName)
{
    server->requestServiceSlot(serviceName);
}

void SenderProcessorThread::unSubscribeServiceSlot(QString serviceName)
{
    server->unSubscribeServiceSlot(serviceName);
}

void SenderProcessorThread::sendCommandMessageSlot(QString commandName, QString message)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(DIALOGCommon::transformMessageFromPlainText(message));

    emit sendCommandMessageSignal(commandName, commandMessage);
}

void SenderProcessorThread::registerCommandSlot(QString commandName)
{
    server->registerCommandSlot(commandName);
}

void SenderProcessorThread::unRegisterCommandSlot(QString commandName)
{
    server->unRegisterCommandSlot(commandName);
}

void SenderProcessorThread::run()
{
    server->connectToControlServerSlot();

    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Sender EventLoop";
}

SenderProcessorThread::~SenderProcessorThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Sender destructor";
}
