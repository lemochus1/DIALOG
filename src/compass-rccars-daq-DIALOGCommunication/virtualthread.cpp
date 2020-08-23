#include "virtualthread.h"

VirtualThread::VirtualThread()
{
    mainThread = currentThread();
    moveToThread(this);
}

void VirtualThread::setServer(Server *serverInit)
{
    server = serverInit;
}

void VirtualThread::messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray* header, QByteArray* message)
{
    DIALOGCommon::logMessage(QString("Received message from (%1, %2): %3, %4")
                            .arg(senderName)
                            .arg(senderPort)
                            .arg(QString(*header))
                            .arg(QString(*message)));
    delete message;
}

void VirtualThread::serverErrorSlot(QString error)
{
}

void VirtualThread::run()
{
    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();
}

void VirtualThread::startThread()
{
    start();
}

void VirtualThread::stop()
{
    virtualThreadEventLoop->exit();
    moveToThread(mainThread);
}
