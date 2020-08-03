#include "socket.h"

Socket::Socket(QObject* parent)
:   QObject(parent)
{
    disconnectionInitiated = false;
    senderKnown = false;
    messageSize = 0;
    messages = NULL;
    server = NULL;
    process = NULL;

    senderAddress = "";
    senderPort = 0;

    receiverAddress = "";
    receiverPort = 0;

    checkIdleSocketTimer = NULL;
    senderTimer = NULL;
}

void Socket::connectToHostSlot()
{
    socket = new QTcpSocket();
    checkIdleSocketTimer = new QTimer();

    QObject::connect(checkIdleSocketTimer, &QTimer::timeout, this, &Socket::checkIdleSocketSlot);
    QObject::connect(socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &Socket::socketErrorSlot);
    QObject::connect(socket, &QTcpSocket::connected, this, &Socket::writeMessageSlot);
    QObject::connect(this, &Socket::newMessageSignal, this, &Socket::writeMessageSlot);
    QObject::connect(this, &Socket::disconnectSocketSignal, this, &Socket::disconnectSocketSlot);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &Socket::removeSocketFromSenderSlot);
    QObject::connect(this, &Socket::deleteSignal, this, &Socket::deleteLater);

    socket->connectToHost(receiverAddress, receiverPort);

    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();
    checkIdleSocketTimer->start(IDLE_SOCKET_TIMER);
}

void Socket::setMessage(MessageContainer* messageContainerInit, QString receiverAddressInit, quint16 receiverPortInit)
{
    if(receiverAddressInit != "" && receiverPortInit != 0)
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

void Socket::disconnectSocketSlot()
{

    if(!disconnectionInitiated)
    {
        disconnectionInitiated = true;
        if(messages != NULL)
            writeMessageSlot();

        Q_EMIT socket->disconnected();
    }
}

void Socket::setSocketSlot()
{
    socket = new QTcpSocket();

    if(server->getProcessType() != ControlServer)
    {
        QObject::connect(this, &Socket::messageReceivedSignal, server->getReceiverThread(), &VirtualThread::messageReceivedSlot/*, Qt::DirectConnection*/);
    }
    QObject::connect(this, &Socket::serverMessageReceivedSignal, server, &Server::messageReceivedSlot);

    QObject::connect(socket, &QTcpSocket::readyRead, this, &Socket::readMessageSlot);
    QObject::connect(this, &Socket::disconnectSocketSignal, this, &Socket::disconnectSocketSlot);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &Socket::removeSocketFromReceiverSlot);
    QObject::connect(this, &Socket::deleteSignal, this, &Socket::deleteLater);

    socket->setSocketDescriptor(socketDescriptor);
    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();
}

void Socket::removeSocketFromSenderSlot()
{
    disconnectionInitiated = true;
    server->processLock.lock();
    if(process != NULL)
    {
        if(process->getReceiverSocket() == this)
            process->setReceiverSocket(NULL);
    }
    server->processLock.unlock();
    Sender* sender = server->getSender();
    sender->removeSocket(this);

    Q_EMIT deleteSignal();
}

void Socket::removeSocketFromReceiverSlot()
{
    disconnectionInitiated = true;
    server->processLock.lock();
    if(process != NULL)
    {
        if(process->getSenderSocket() == this)
            process->setSenderSocket(NULL);
    }
    server->processLock.unlock();
    Receiver* receiver = server->getReceiver();
    receiver->removeSocket(this);

    Q_EMIT deleteSignal();
}

void Socket::checkIdleSocketSlot()
{
    if(QDateTime::currentMSecsSinceEpoch() - IDLE_SOCKET_TIMER > lastActionTimeStamp)
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
    if(senderTimer == NULL)
    {
        senderTimer = new QTimer();
        QObject::connect(senderTimer, &QTimer::timeout, this, &Socket::writeMessageSlot);
        senderTimer->start(SEND_SOCKET_TIMER);
    }

    while(!messages->isEmpty())
    {
        lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();

        MessageContainer* message = messages->pop();

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint32)0;

        if(!senderKnown)
        {
            senderKnown = true;
            QByteArray sender;
            sender.append(server->getServerProcess()->processAddress);
            sender.append(SEPARATOR);
            sender.append(QString::number(server->getServerProcess()->processPort));
            out << sender;
        }
        if(message->getMessage() == NULL)
        {
            out << quint32(message->getHeader()->size());
            out << *message->getHeader();
        }
        else
        {
            out << quint32(message->getHeader()->size() + message->getMessage()->size());
            out << *message->getHeader();
            out << *message->getMessage();
        }

        out.device()->seek(0);
        messageSize = block.size() - sizeof(quint32);
        out << quint32(messageSize);
        socket->write(block);

        if(message->deleteMessage() == 0)
            delete message;
    }
}

void Socket::readMessageSlot()
{
    lastActionTimeStamp = QDateTime::currentMSecsSinceEpoch();

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_0);

    while(socket->bytesAvailable() > 0)
    {
        if (messageSize == 0) {
            if (socket->bytesAvailable() < (int)sizeof(quint32))
                return;

            in >> messageSize;
        }

        if (socket->bytesAvailable() < messageSize)
            return;

        if(messageSize > 0)
        {
            if(!senderKnown)
            {
                senderKnown = true;
                QByteArray sender;
                in >> sender;
                QList<QByteArray> senderList = sender.split(SEPARATOR);
                senderAddress = senderList[0];
                senderPort = senderList[1].toUShort();
                QString senderKey = senderAddress + SEPARATOR + QString::number(senderPort);
                Receiver* receiver = server->getReceiver();
                receiver->addSocket(this);
                if(server->getControlServer()->processKey == senderKey)
                {
                    server->getControlServer()->setSenderSocket(this);
                    process = server->getControlServer();
                }
                else
                {
                    if(!server->isProcessKnown(senderKey))
                    {
                        Process* process = new Process(senderAddress, senderPort);
                        server->addProcess(senderKey, process);
                    }
                    server->getProcess(senderKey)->setSenderSocket(this);
                    process = server->getProcess(senderKey);
                }
            }

            quint32 subMessageSize = 0;
            QByteArray* subHeader = new QByteArray();
            in >> subMessageSize;
            in >> *subHeader;

            if(server->getProcessType() == ControlServer)
            {
                if(subHeader->contains(LIST_OF_AVAILABLE_SERVICES) || subHeader->contains(HEARTBEAT))
                {
                    if(subMessageSize > 0 && subHeader->size() == subMessageSize)
                    {
                        Q_EMIT serverMessageReceivedSignal(senderAddress, senderPort, subHeader);
                    }
                    else
                    {
                        delete subHeader;
                    }
                }
                else
                {
                    QByteArray* subMessage = new QByteArray();
                    in >> *subMessage;

                    if(subMessageSize > 0 && (subHeader->size() + subMessage->size()) == subMessageSize)
                    {
                        Q_EMIT serverMessageReceivedSignal(senderAddress, senderPort, subHeader, subMessage);
                    }
                    else
                    {
                        delete subHeader;
                        delete subMessage;
                    }
                }
            }
            else
            {
                if(subHeader->contains(SUCCESSFULY_CONNECTED) || subHeader->contains(CONNECTION_LOST))
                {
                    if(subMessageSize > 0 && subHeader->size() == subMessageSize)
                    {
                        Q_EMIT serverMessageReceivedSignal(senderAddress, senderPort, subHeader);
                    }
                    else
                    {
                        delete subHeader;
                    }
                }
                else
                {
                    QByteArray* subMessage = new QByteArray();
                    in >> *subMessage;

                    if(subMessageSize > 0 && (subHeader->size() + subMessage->size()) == subMessageSize)
                    {
                        if(subHeader->contains(SERVICE_MESSAGE) || subHeader->contains(COMMAND_MESSAGE) || subHeader->contains(PROCEDURE_MESSAGE) || subHeader->contains(LIST_OF_AVAILABLE_SERVICES) || subHeader->contains(INFO_MONITORING))
                        {
                            Q_EMIT messageReceivedSignal(senderAddress, senderPort, subHeader, subMessage);
                        }
                        else
                        {
                            Q_EMIT serverMessageReceivedSignal(senderAddress, senderPort, subHeader, subMessage);
                        }
                    }
                    else
                    {
                        delete subHeader;
                        delete subMessage;
                    }
                }
            }
        }
        messageSize = 0;
    }
}

Socket::~Socket()
{
    if(senderTimer != NULL)
    {
        senderTimer->stop();
        delete senderTimer;
    }

    if(checkIdleSocketTimer != NULL)
    {
        checkIdleSocketTimer->stop();
        delete checkIdleSocketTimer;
    }

    if(messages != NULL)
    {
        while(!messages->isEmpty())
        {
            MessageContainer* message = messages->pop();
            if(message->deleteMessage() == 0)
                delete message;
        }

        delete messages;
    }

    delete socket;
}
