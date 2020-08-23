#include "sender.h"

Sender::Sender(Server* serverInit)
{
    senderStarted = false;

    serverThread = currentThread();

    server = serverInit;

    numberOfOpenSockets = 0;

    moveToThread(this);
}

void Sender::sendMessageSlot(QString receiverAddress,
                             quint16 receiverPort,
                             QByteArray* header,
                             QByteArray* message)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(header, message);
        QString receiverKey = receiverAddress + SEPARATOR + QString::number(receiverPort);
        Process* process;
        if(server->getControlServer()->processKey == receiverKey)
            process = server->getControlServer();
        else
            process = server->getProcess(receiverKey);

        if(!process->sendMessage(messageContainer))
            sendToHost(receiverAddress, receiverPort, messageContainer);
    }
    else
        delete message;
}

void Sender::sendHeartBeatSlot(QByteArray* header)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(header);
        if(!server->getControlServer()->sendMessage(messageContainer))
            sendToHost(server->getControlServer()->processAddress,
                       server->getControlServer()->processPort,
                       messageContainer);
    }
}

void Sender::sendServiceMessageSlot(QString serviceName, QByteArray* message)
{
    if(senderStarted)
    {
        Service* service = server->getService(serviceName);
        if(service != NULL)
        {
            service->receiversLock.lock();
            MessageContainer* messageContainer = new MessageContainer(
                                                        server->messageHeader(
                                                                QString(SERVICE_MESSAGE)
                                                                .append(SEPARATOR)
                                                                .append(serviceName)),
                                                        message,
                                                        service->receivers.count());
            foreach(Process* receiver, service->receivers)
            {
                if(!receiver->sendMessage(messageContainer))
                    sendToHost(receiver->processAddress, receiver->processPort, messageContainer);
            }
            service->receiversLock.unlock();
        }
        else
            DIALOGCommon::logMessage(QString("Service %1 is not provided on the server.")
                                     .arg(serviceName));
    }
    else
        delete message;
}

void Sender::sendCommandMessageSlot(QString commandName, QByteArray* message)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(
                                                server->messageHeader(QString(COMMAND_MESSAGE)
                                                                      .append(SEPARATOR)
                                                                      .append(commandName)),
                                                message);
        if(!server->getControlServer()->sendMessage(messageContainer))
            sendToHost(server->getControlServer()->processAddress,
                       server->getControlServer()->processPort,
                       messageContainer);
    }
    else
        delete message;
}

void Sender::sendDirectCommandMessageSlot(QString commandName,
                                          QByteArray *message,
                                          QString processName)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(
                                                    server->messageHeader(QString(COMMAND_MESSAGE)
                                                                          .append(SEPARATOR)
                                                                          .append(DIRECT)
                                                                          .append(SEPARATOR)
                                                                          .append(processName)
                                                                          .append(SEPARATOR)
                                                                          .append(commandName)),
                                                    message);
        if(!server->getControlServer()->sendMessage(messageContainer))
            sendToHost(server->getControlServer()->processAddress,
                       server->getControlServer()->processPort,
                       messageContainer);
    }
    else
        delete message;
}

void Sender::sendDirectCommandUrlMessageSlot(QString commandName,
                                             QByteArray *message,
                                             QString url,
                                             int port)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(
                                                    server->messageHeader(QString(COMMAND_MESSAGE)
                                                                          .append(SEPARATOR)
                                                                          .append(commandName)),
                                                    message);
        QString senderProcessKey = url + SEPARATOR + QString::number(port);
        if(!server->isProcessKnown(senderProcessKey))
        {
            Process* senderProcess = new Process(url, port, Custom);
            server->addProcess(senderProcessKey, senderProcess);
        }
        sendToHost(url, port, messageContainer);
    }
    else
        delete message;
}

void Sender::callProcedureMessageSlot(QString procedureName, QByteArray *message)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(
                                                    server->messageHeader(QString(PROCEDURE_MESSAGE)
                                                                          .append(SEPARATOR)
                                                                          .append(PROCEDURE_CALL)
                                                                          .append(SEPARATOR)
                                                                          .append(procedureName)),
                                                    message);
        if(!server->getControlServer()->sendMessage(messageContainer))
            sendToHost(server->getControlServer()->processAddress,
                       server->getControlServer()->processPort,
                       messageContainer);
    }
    else
        delete message;
}

void Sender::sendProcedureReturnMessageSlot(QString procedureName,
                                            QByteArray *message,
                                            QString url,
                                            int port)
{
    if(senderStarted)
    {
        MessageContainer* messageContainer = new MessageContainer(
                                                    server->messageHeader(QString(PROCEDURE_MESSAGE)
                                                                          .append(SEPARATOR)
                                                                          .append(PROCEDURE_DATA)
                                                                          .append(SEPARATOR)
                                                                          .append(procedureName)),
                                                    message);
        QString senderProcessKey = url + SEPARATOR + QString::number(port);
        if(!server->isProcessKnown(senderProcessKey))
        {
            Process* senderProcess = new Process(url, port, Custom);
            server->addProcess(senderProcessKey, senderProcess);
        }
        sendToHost(url, port, messageContainer);
    }
    else
        delete message;
}

void Sender::senderErrorSlot(QString error)
{
    Q_EMIT senderErrorSignal(error);
}

bool Sender::anyOpenSockets()
{
    return (numberOfOpenSockets > 0);
}

void Sender::closeAllSockets()
{
    processSocketLock.lock();
    foreach (Socket* socket, sockets) {
        Q_EMIT socket->disconnectSocketSignal();
    }
    processSocketLock.unlock();
}

void Sender::closeSocket(Socket* socket)
{
    if(socket != NULL)
        Q_EMIT socket->disconnectSocketSignal();
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
    if(sockets.size() > 0)
        sockets.removeAll(socket);
    processSocketLock.unlock();
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

    if(senderStarted && numberOfOpenSockets <= MAXIMUM_OF_PENDING_SENDER_CONNECTIONS)
    {
        Socket* newSocket = new Socket();
        addSocket(newSocket);
        if(server->getControlServer()->processKey == receiverKey)
        {
            server->getControlServer()->setReceiverSocket(newSocket);
            newSocket->setProcess(server->getControlServer());
        }
        else
        {
            Process* process = server->getProcess(receiverKey);
            if (!process)
            {
                DIALOGCommon::logMessage("Try to send to not known process" + receiverKey);
                if(messageContainer->deleteMessage() == 0)
                    delete messageContainer;
                return;
            }

            process->setReceiverSocket(newSocket);
            newSocket->setProcess(process);
        }

        newSocket->moveToThread(senderThreads[currentThreadIndex]);
        currentThreadIndex++;
        if(currentThreadIndex == NUMBER_OF_SENDER_THREADS)
            currentThreadIndex = 0;
        numberOfOpenSockets++;

        newSocket->setServer(server);
        newSocket->setMessage(messageContainer, receiverAddress, receiverPort);
        QObject::connect(this, &Sender::connectToHostSignal, newSocket, &Socket::connectToHostSlot);
        QObject::connect(newSocket, &Socket::socketErrorSignal, this, &Sender::senderErrorSlot);
        QObject::connect(newSocket, &Socket::destroyed, this, &Sender::socketDisconnectedSlot);

        Q_EMIT connectToHostSignal();
        QObject::disconnect(this, &Sender::connectToHostSignal,
                            newSocket, &Socket::connectToHostSlot);
    }
    else
    {
        if(messageContainer->deleteMessage() == 0)
            delete messageContainer;
    }
}

Sender::~Sender()
{
    DIALOGCommon::logMessage("ServerSender destructor");
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

    senderEventLoop = new QEventLoop();
    senderEventLoop->exec();

    DIALOGCommon::logMessage("End of ServerSender EventLoop");
}

void Sender::startThread()
{
    start();
}

void Sender::stop()
{
    senderStarted = false;

    while(anyOpenSockets())
    {
        //closeAllSockets();
        senderEventLoop->processEvents();
    }

    for(int i = 0; i < NUMBER_OF_SENDER_THREADS; i++)
        senderThreads[i]->quit();

    senderEventLoop->exit();
    moveToThread(serverThread);
}
