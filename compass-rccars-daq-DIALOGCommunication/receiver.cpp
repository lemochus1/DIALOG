#include "receiver.h"

Receiver::Receiver(QString receiverAddressInit, quint16 receiverPortInit, Server* serverInit)
{
    receiverStarted = false;

    serverThread = currentThread();

    receiverAddress = receiverAddressInit;
    receiverPort = receiverPortInit;

    server = serverInit;

    numberOfOpenSockets = 0;

    moveToThread(this);
}

QString Receiver::getReceiverAddress()
{
    receiverAddress = tcpServer->getReceiverAddress();
    return receiverAddress;
}

quint16 Receiver::getReceiverPort()
{
    receiverPort = tcpServer->getReceiverPort();
    return receiverPort;
}

void Receiver::run()
{
    tcpServer = new TcpServer(receiverAddress, receiverPort, this);

    receiverStarted = true;
    Q_EMIT tcpServerStartedSignal();

    currentThreadIndex = 0;
    for(int i = 0; i < NUMBER_OF_RECEIVER_THREADS; i++)
    {
        receiverThreads[i] = new QThread();
        QObject::connect(receiverThreads[i], &QThread::finished, receiverThreads[i], &QThread::deleteLater);

        receiverThreads[i]->start();
    }

    receiverEventLoop = new QEventLoop();
    receiverEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of ServerReceiver EventLoop";
}

void Receiver::stop()
{
    receiverStarted = false;
    tcpServer->close();

    while(anyOpenSockets())
    {
        closeAllSockets();
        receiverEventLoop->processEvents();
    }

    for(int i = 0; i < NUMBER_OF_RECEIVER_THREADS; i++)
        receiverThreads[i]->quit();

    delete tcpServer;

    receiverEventLoop->exit();
    moveToThread(serverThread);
}

bool Receiver::anyOpenSockets()
{
    return (numberOfOpenSockets > 0);
}

void Receiver::closeAllSockets()
{
    processSocketLock.lock();
    foreach (Socket* socket, sockets) {
        Q_EMIT socket->disconnectSocketSignal();
    }
    processSocketLock.unlock();
}

void Receiver::closeSocket(Socket* socket)
{
    if(socket != NULL)
        Q_EMIT socket->disconnectSocketSignal();
}

void Receiver::addSocket(Socket *socket)
{
    processSocketLock.lock();
    sockets.append(socket);
    processSocketLock.unlock();
}

void Receiver::removeSocket(Socket *socket)
{
    processSocketLock.lock();
    if(sockets.size() > 0)
        sockets.removeAll(socket);
    processSocketLock.unlock();
}

void Receiver::socketDisconnectedSlot()
{
    numberOfOpenSockets--;
}

void Receiver::startThread()
{
    start();
}

Receiver::~Receiver()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "ServerReceiver destructor";
}
