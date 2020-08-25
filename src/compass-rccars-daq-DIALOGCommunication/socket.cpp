#include "socket.h"

Socket::Socket(QObject* parent)
:   QObject(parent)
{
    disconnectionInitiated = false;
    senderKnown = false;
    messages = nullptr;
    server = nullptr;
    process = nullptr;

    senderAddress = "";
    senderPort = 0;

    receiverAddress = "";
    receiverPort = 0;

    messageSize = 0;

    checkIdleSocketTimer = nullptr;
    senderTimer = nullptr;
}

void Socket::connectToHostSlot()
{
    socket = new QTcpSocket();
    checkIdleSocketTimer = new QTimer();

    QObject::connect(checkIdleSocketTimer, &QTimer::timeout,
                     this, &Socket::checkIdleSocketSlot);
    QObject::connect(socket,
                     static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
                     this, &Socket::socketErrorSlot);
    QObject::connect(socket, &QTcpSocket::connected,
                     this, &Socket::writeMessageSlot);
    QObject::connect(this, &Socket::newMessageSignal,
                     this, &Socket::writeMessageSlot);
    QObject::connect(this, &Socket::disconnectSocketSignal,
                     this, &Socket::disconnectSocketSlot);
    QObject::connect(socket, &QTcpSocket::disconnected,
                     this, &Socket::removeSocketFromSenderSlot);
    QObject::connect(this, &Socket::deleteSignal,
                     this, &Socket::deleteLater);

    socket->connectToHost(receiverAddress, receiverPort);

    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();
    checkIdleSocketTimer->start(IDLE_SOCKET_TIMER);
}

void Socket::setMessage(MessageContainer* messageContainerInit,
                        QString receiverAddressInit,
                        quint16 receiverPortInit)
{
    if (!receiverAddressInit.isEmpty() && receiverPortInit)
    {
        messages = new ThreadSafeQList();
        receiverAddress = receiverAddressInit;
        receiverPort = receiverPortInit;
    }

    messages->push(messageContainerInit);
}

void Socket::setSocketDescriptor(qintptr socketDescriptorInit)
{
    socketDescriptor = socketDescriptorInit;
}

void Socket::setServer(Server *serverInit)
{
    server = serverInit;
}

void Socket::setProcess(Process *processInit)
{
    process = processInit;
}

void Socket::setSender(const QByteArray &sender)
{
    QList<QByteArray> senderList = sender.split(SEPARATOR);
    senderAddress = senderList[0];
    senderPort = senderList[1].toUShort();

    server->getReceiver()->addSocket(this);

    process = server->getProcess(senderAddress, senderPort);
    process->setSenderSocket(this);
}

QByteArray Socket::getSender()
{
    QByteArray sender;
    sender.append(server->getServerProcess()->processAddress);
    sender.append(SEPARATOR);
    sender.append(QString::number(server->getServerProcess()->processPort));
    return sender;
}

void Socket::disconnectSocketSlot()
{
    if (!disconnectionInitiated)
    {
        disconnectionInitiated = true;
        if (messages != nullptr)
            writeMessageSlot();
        Q_EMIT socket->disconnected();
    }
}

void Socket::setSocketSlot()
{
    socket = new QTcpSocket();

    if (server->getProcessType() != ControlServer)
    {
        QObject::connect(this, &Socket::messageReceivedSignal,
                         server->getReceiverThread(), &VirtualThread::messageReceivedSlot);
    }
    QObject::connect(this, &Socket::serverMessageReceivedSignal,
                     server, &Server::messageReceivedSlot);

    QObject::connect(socket, &QTcpSocket::readyRead,
                     this, &Socket::readMessageSlot);
    QObject::connect(this, &Socket::disconnectSocketSignal,
                     this, &Socket::disconnectSocketSlot);
    QObject::connect(socket, &QTcpSocket::disconnected,
                     this, &Socket::removeSocketFromReceiverSlot);
    QObject::connect(this, &Socket::deleteSignal,
                     this, &Socket::deleteLater);

    socket->setSocketDescriptor(socketDescriptor);
    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();
}

void Socket::removeSocketFromSenderSlot()
{
    disconnectionInitiated = true;
    server->removeReceiverSocket(process, this);
    server->getSender()->removeSocket(this);
    Q_EMIT deleteSignal();
}

void Socket::removeSocketFromReceiverSlot()
{
    disconnectionInitiated = true;
    server->removeReceiverSocket(process, this);
    server->getReceiver()->removeSocket(this);
    Q_EMIT deleteSignal();
}

void Socket::checkIdleSocketSlot()
{
    if (QDateTime::currentMSecsSinceEpoch() - IDLE_SOCKET_TIMER > lastActionTimeStamp)
        disconnectSocketSlot();
}

void Socket::socketErrorSlot(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError)
    {
        QString connectionError;
        connectionError.append(CONNECTION_ERROR);
        connectionError.append(SEPARATOR);

        switch (socketError) {
        case QAbstractSocket::HostNotFoundError:
            connectionError.append(HOST_NOT_FOUND_ERROR);
            break;
        case QAbstractSocket::ConnectionRefusedError:
            connectionError.append(CONNECTION_REFUSED_ERROR);
            break;
        default:
            connectionError.append(DEFAULT_ERROR);
            connectionError.append(SEPARATOR);
            connectionError.append(socket->errorString());
        }

        connectionError.append(SEPARATOR);
        connectionError.append(receiverAddress);
        connectionError.append(SEPARATOR);
        connectionError.append(QString::number(receiverPort));

        Q_EMIT socketErrorSignal(connectionError);
    }

    disconnectSocketSlot();
}

void Socket::writeMessageSlot()
{
    if (!senderTimer)
    {
        senderTimer = new QTimer();
        QObject::connect(senderTimer, &QTimer::timeout,
                         this, &Socket::writeMessageSlot);
        senderTimer->start(SEND_SOCKET_TIMER);
    }

    while (!messages->isEmpty())
    {
        lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();

        MessageContainer* message = messages->pop();
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint32)0;

        if (!senderKnown)
        {
            out << getSender();
            senderKnown = true;
        }

        out << message->getSize();
        out << *message->getHeader();
        if (message->getMessage())
        {
            out << *message->getMessage();
        }

        out.device()->seek(0);
        messageSize = block.size() - sizeof(quint32);
        out << messageSize;
        socket->write(block);

        if (message->deleteMessage() == 0)
            delete message;
    }
}

void Socket::readMessageSlot()
{
    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_0);

    while (socket->bytesAvailable() > 0)
    {
        if (messageSize == 0) {
            if (socket->bytesAvailable() < (int)sizeof(quint32))
                return;
            in >> messageSize;
        }
        if (socket->bytesAvailable() < messageSize)
            return;

        if (messageSize > 0)
        {
            if (!senderKnown)
            {
    //            Nekdy to spadne
                QByteArray sender;
                in >> sender;
                setSender(sender);
                senderKnown = true;
            }

            quint32 subMessageSize = 0;
            in >> subMessageSize;
            if (subMessageSize <= 0)
            {
                break;
            }

            QByteArray* subHeader = new QByteArray();
            in >> *subHeader;
    
            QByteArray* subMessage = nullptr;
            quint32 rightSize = subHeader->size();
    
            if (!subHeader->contains(LIST_OF_AVAILABLE_SERVICES) &&
               !subHeader->contains(HEARTBEAT) &&
               !subHeader->contains(SUCCESSFULY_CONNECTED) &&
               !subHeader->contains(CONNECTION_LOST))
            {
                subMessage = new QByteArray();
                in >> *subMessage;
                rightSize += subMessage->size();
            }
    
            if (rightSize == subMessageSize) {
                if (server->getProcessType() != ControlServer &&
                   (subHeader->contains(SERVICE_MESSAGE) ||
                   subHeader->contains(COMMAND_MESSAGE) ||
                   subHeader->contains(PROCEDURE_MESSAGE) ||
                   subHeader->contains(LIST_OF_AVAILABLE_SERVICES) ||
                   subHeader->contains(INFO_MONITORING)))
                {
                    Q_EMIT messageReceivedSignal(senderAddress,
                                                 senderPort,
                                                 subHeader,
                                                 subMessage);
                }
                else {
                    Q_EMIT serverMessageReceivedSignal(senderAddress,
                                                       senderPort,
                                                       subHeader,
                                                       subMessage);
                }
            }
            else {
                delete subHeader;
                if (subMessage)
                {
                    delete subMessage;
                }
            }
            messageSize = 0;
        }
    }
}

Socket::~Socket()
{
    if (senderTimer)
    {
        senderTimer->stop();
        delete senderTimer;
    }
    if (checkIdleSocketTimer)
    {
        checkIdleSocketTimer->stop();
        delete checkIdleSocketTimer;
    }
    if (messages)
    {
        while (!messages->isEmpty())
        {
            MessageContainer* message = messages->pop();
            if (message->deleteMessage() == 0)
                delete message;
        }
        delete messages;
    }
    delete socket;
}
