#include "sender.h"

Sender::Sender(Server* serverInit)
    : senderStarted(false),
      numberOfOpenSockets(0)
{
    serverThread = currentThread();
    server = serverInit;

    moveToThread(this);
}

Sender::~Sender()
{
    DIALOGCommon::logMessage("ServerSender destructor");
}

bool Sender::anyOpenSockets()
{
    return (numberOfOpenSockets > 0);
}

void Sender::closeAllSockets()
{
    processSocketLock.lock();
    foreach (Socket* socket, sockets)
    {
        Q_EMIT socket->disconnectSocketSignal();
    }
    processSocketLock.unlock();
}

void Sender::closeSocket(Socket* socket)
{
    if (socket)
    {
        Q_EMIT socket->disconnectSocketSignal();
    }
}

void Sender::addSocket(Socket *socket)
{
    processSocketLock.lock();
    sockets.append(socket);
    processSocketLock.unlock();
}

void Sender::removeSocket(Socket *socket)
{
    processSocketLock.lock();
    if (sockets.size() > 0)
    {
        sockets.removeAll(socket);
    }
    processSocketLock.unlock();
}

void Sender::run()
{
    currentThreadIndex = 0;
    for(int i = 0; i < NUMBER_OF_SENDER_THREADS; i++)
    {
        senderThreads[i] = new QThread();
        QObject::connect(senderThreads[i], &QThread::finished,
                         senderThreads[i], &QThread::deleteLater);
        senderThreads[i]->start();
    }

    senderStarted = true;
    Q_EMIT senderStartedSignal();

    senderEventLoop = new QEventLoop(this);
    senderEventLoop->exec();

    DIALOGCommon::logMessage("End of ServerSender EventLoop");
}


void Sender::sendMessageSlot(QString receiverAddress,
                             quint16 receiverPort,
                             QByteArray* header,
                             QByteArray* message)
{
    Process* process = server->getProcess(receiverAddress, receiverPort, false);
    sendMessageImpl(process, header, message);
}

void Sender::sendHeartBeatSlot(QByteArray* header)
{
    sendToControlServer(header);
}

void Sender::sendServiceMessageSlot(QString serviceName, QByteArray* message)
{
    sendToServiceReceivers(serviceName,
                           createHeader({SERVICE_MESSAGE, serviceName}),
                           message);
}

void Sender::sendServicePublisherMessageSlot(QString serviceName, QByteArray *message)
{
    sendToServiceReceivers(serviceName,
                           createHeader({SERVICE_MESSAGE,
                                         serviceName,
                                         PUBLISHER_MESSAGE}),
                           message);
}

void Sender::sendServiceDataRequestMessageSlot(QString serviceName)
{
    sendToServiceSender(serviceName, createHeader({SERVICE_MESSAGE,
                                                   serviceName,
                                                   UPDATE_REQUEST}));
}

void Sender::sendServiceSubscriberMessageSlot(QString serviceName, QByteArray *message)
{
    sendToServiceSender(serviceName,
                        createHeader({SERVICE_MESSAGE,
                                      serviceName,
                                      SUBSCRIBER_MESSAGE}),
                        message);
}

void Sender::sendCommandMessageSlot(QString commandName, QByteArray* message)
{
    sendToControlServer(createHeader({COMMAND_MESSAGE,
                                     commandName}),
                        message);
}

void Sender::sendCommandNameMessageSlot(QString commandName,
                                        QByteArray *message,
                                        QString processName)
{
    sendToControlServer(createHeader({COMMAND_MESSAGE,
                                      DIRECT,
                                      processName,
                                      commandName}),
                        message);
}

void Sender::sendCommandAddressMessageSlot(QString commandName,
                                           QByteArray *message,
                                           QString address,
                                           int port)
{
    Process* targetProcess = server->getProcess(address, port);
    sendMessageImpl(targetProcess, createHeader({COMMAND_MESSAGE, commandName}), message);
}

void Sender::callProcedureMessageSlot(QString procedureName, QByteArray *message, int callerId)
{
    sendToControlServer(createHeader({PROCEDURE_MESSAGE,
                                      PROCEDURE_CALL,
                                      procedureName,
                                      QString::number(callerId)}),
                        message);
}

void Sender::callProcedureNameMessageSlot(QString procedureName,
                                          QByteArray *message,
                                          QString processName,
                                          int callerId)
{
    sendToControlServer(createHeader({PROCEDURE_MESSAGE,
                                      PROCEDURE_CALL,
                                      DIRECT,
                                      processName,
                                      procedureName,
                                      QString::number(callerId)}),
                        message);
}

void Sender::callProcedureAddressMessageSlot(QString procedureName,
                                             QByteArray *message,
                                             QString address,
                                             int port,
                                             int callerId)
{
    sendProcedureMessageImpl(procedureName, PROCEDURE_CALL, address, port, callerId, message);
}

void Sender::sendProcedureReturnMessageSlot(QString procedureName,
                                            QByteArray *message,
                                            QString address,
                                            int port,
                                            int callerId)
{
    sendProcedureMessageImpl(procedureName, PROCEDURE_DATA, address, port, callerId, message);
}

void Sender::sendProcedureFailedMessageSlot(QString procedureName,
                                            QByteArray *message,
                                            QString address,
                                            int port,
                                            int callerId)
{
    sendProcedureMessageImpl(procedureName, PROCEDURE_FAILED, address, port, callerId, message);
}

void Sender::sendProcedureInvalidParamsMessageSlot(QString procedureName,
                                                   QString address,
                                                   int port,
                                                   int callerId)
{
    sendProcedureMessageImpl(procedureName, PROCEDURE_INVALID, address, port, callerId);
}

void Sender::senderErrorSlot(QString error)
{
    Q_EMIT senderErrorSignal(error);
}

void Sender::socketDisconnectedSlot()
{
    numberOfOpenSockets--;
}

void Sender::sendToHost(QString receiverAddress,
                        quint16 receiverPort,
                        MessageContainer* messageContainer)
{
    QString receiverKey = receiverAddress + SEPARATOR + QString::number(receiverPort);
    if (senderStarted && numberOfOpenSockets <= MAXIMUM_OF_PENDING_SENDER_CONNECTIONS)
    {
        Socket* newSocket = new Socket();
        addSocket(newSocket);

        Process* process = server->getProcess(receiverAddress, receiverPort, false);
        if (!process)
        {
            DIALOGCommon::logMessage("Try to send to not known process: " + receiverKey);
            if (messageContainer->deleteMessage() == 0)
                delete messageContainer;
            return;
        }
        process->setReceiverSocket(newSocket);
        newSocket->setProcess(process);

        newSocket->moveToThread(senderThreads[currentThreadIndex]);
        currentThreadIndex++;
        if (currentThreadIndex == NUMBER_OF_SENDER_THREADS)
            currentThreadIndex = 0;
        numberOfOpenSockets++;

        newSocket->setServer(server);
        newSocket->setMessage(messageContainer, receiverAddress, receiverPort);
        QObject::connect(this, &Sender::connectToHostSignal,
                         newSocket, &Socket::connectToHostSlot);
        QObject::connect(newSocket, &Socket::socketErrorSignal,
                         this, &Sender::senderErrorSlot);
        QObject::connect(newSocket, &Socket::destroyed,
                         this, &Sender::socketDisconnectedSlot);

        Q_EMIT connectToHostSignal();
        QObject::disconnect(this, &Sender::connectToHostSignal,
                            newSocket, &Socket::connectToHostSlot);
    }
    else
    {
        if (messageContainer->deleteMessage() == 0)
            delete messageContainer;
    }
}

void Sender::startThread()
{
    start();
}

void Sender::stop()
{
    senderStarted = false;

    while (anyOpenSockets())
    {
        senderEventLoop->processEvents();
    }

    for(int i = 0; i < NUMBER_OF_SENDER_THREADS; i++)
    {
        senderThreads[i]->quit();
    }

    senderEventLoop->exit();
    moveToThread(serverThread);
}

bool Sender::canBeSent(QByteArray *message)
{
    if (!senderStarted)
    {
        DIALOGCommon::logMessage("Message could not be sent. Sender was not started.");
        if (message)
        {
            delete message;
        }
        return false;
    }
    return true;
}

void Sender::sendMessageImpl(Process *targetProcess, MessageContainer *messageContainer)
{
    if (targetProcess)
    {
        if (!targetProcess->sendMessage(messageContainer))
        {
            sendToHost(targetProcess->processAddress, targetProcess->processPort, messageContainer);
        }
        return;
    }
    DIALOGCommon::logMessage("Message could not be sent. Invalid process forwarded.");
}

void Sender::sendMessageImpl(Process *targetProcess,
                             QByteArray* header,
                             QByteArray *message,
                             quint32 messageCounterInit)
{
    if (canBeSent(message))
    {
        MessageContainer* messageContainer = new MessageContainer(header,
                                                                  message,
                                                                  messageCounterInit);
        sendMessageImpl(targetProcess, messageContainer);
    }
}

void Sender::sendToControlServer(QByteArray* header,
                                 QByteArray *message,
                                 quint32 messageCounterInit)
{
    Process* process = server->getControlServer();
    if (process)
    {
        sendMessageImpl(process, header, message, messageCounterInit);
        return;
    }
    DIALOGCommon::logMessage("Message could not be sent. Control server is unknown.");
}

void Sender::sendToServiceSender(const QString serviceName,
                                 QByteArray *header,
                                 QByteArray *message)
{
    Service* service = server->getService(serviceName);
    if (service)
    {
        sendMessageImpl(service->sender, header, message);
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 is not provided on the server.")
                                 .arg(serviceName));
}

void Sender::sendToServiceReceivers(const QString serviceName,
                                    QByteArray *header,
                                    QByteArray *message)
{
    if (canBeSent(message))
    {
        Service* service = server->getService(serviceName);
        if (service)
        {
            service->receiversLock.lock();
            MessageContainer* messageContainer = new MessageContainer(header,
                                                                      message,
                                                                      service->receivers.count());
            foreach(Process* receiver, service->receivers)
            {
                sendMessageImpl(receiver, messageContainer);
            }
            service->receiversLock.unlock();
        }
        else
            DIALOGCommon::logMessage(QString("Service %1 is not provided on the server.")
                                     .arg(serviceName));
    }
}

void Sender::sendProcedureMessageImpl(QString procedureName,
                                      QString procedureMessageType,
                                      QString address,
                                      int port,
                                      int callerId,
                                      QByteArray *message)
{
    Process* targetProcess = server->getProcess(address, port);
    sendMessageImpl(targetProcess,
                    createHeader({PROCEDURE_MESSAGE,
                                  procedureMessageType,
                                  procedureName,
                                  QString::number(callerId)}),
                    message);
}

QString Sender::createHeaderKey(const QStringList& headerList)
{
    return headerList.join(SEPARATOR);
}

QByteArray *Sender::createHeader(const QStringList &headerList)
{
    return server->createMessageHeader(createHeaderKey(headerList));
}
