#include "tcpserver.h"

TcpServer::TcpServer(QString receiverAddressInit, quint16 receiverPortInit, Receiver* receiverInit, QObject *parent)
:   QTcpServer(parent)
{
    receiverAddress = receiverAddressInit;

    receiver = receiverInit;

    sessionOpened(receiverPortInit);
}

TcpServer::~TcpServer()
{
}

QString TcpServer::getReceiverAddress()
{
    return receiverAddress;
}

quint16 TcpServer::getReceiverPort()
{
    return receiverPort;
}

void TcpServer::sessionOpened(quint16 receiverPortInit)
{
    setMaxPendingConnections(MAXIMUM_OF_PENDING_RECEIVER_CONNECTIONS);

    if (!listen(QHostAddress::Any, receiverPortInit)) {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unable to start the server: " << errorString();

        return;
    }
    receiverPort = serverPort();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The server is running on IP: " << receiverAddress << ", port: " << receiverPort;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    if(receiver->receiverStarted)
    {
        Socket *socket = new Socket();
        socket->moveToThread(receiver->receiverThreads[receiver->currentThreadIndex]);
        receiver->currentThreadIndex++;
        if(receiver->currentThreadIndex == NUMBER_OF_RECEIVER_THREADS)
            receiver->currentThreadIndex = 0;
        receiver->numberOfOpenSockets++;

        socket->setServer(receiver->server);
        socket->setSocketDescriptor(socketDescriptor);
        QObject::connect(receiver, &Receiver::setSocketSignal, socket, &Socket::setSocketSlot);
        QObject::connect(socket, &Socket::destroyed, receiver, &Receiver::socketDisconnectedSlot, Qt::DirectConnection);

        Q_EMIT receiver->setSocketSignal();
        QObject::disconnect(receiver, &Receiver::setSocketSignal, socket, &Socket::setSocketSlot);
    }
}
