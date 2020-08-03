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
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Received message from (" << senderName << ", " << senderPort << "): " << *header << *message << ".";

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
