#include "senderprocessorthread.h"

void SenderProcessorThread::requestServiceSlot(QString serviceName)
{
    if(!server->isServiceKnown(serviceName))
    {
        server->requestServiceSlot(serviceName);
    }
}

void SenderProcessorThread::registerCommandSlot(QString commandName)
{
    if(!server->isCommandKnown(commandName))
    {
        server->registerCommandSlot(commandName);
    }
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
