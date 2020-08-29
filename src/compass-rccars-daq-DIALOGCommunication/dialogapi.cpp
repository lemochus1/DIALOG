#include "dialogapi.h"

// Time before communication attempts are cancelled.
const int CONNECT_TIMEOUT = 5; // seconds

//==================================================================================================
// Help functions
//==================================================================================================

static void deleteLater(QObject *obj)
{
    if (obj)
    {
        obj->deleteLater();
    }
}

template<typename T>
static QString tryGetName(QWeakPointer<T> handler)
{
    auto strongHandler = handler.toStrongRef();
    if (!strongHandler)
    {
        qWarning() << "Handler name could not be extracted! Invalid QWeakPointer forwarded.";
        return "";
    }
    return strongHandler->getName();
}

template<typename T>
static bool invokeMethod(QWeakPointer<T> handler, const char *method)
{
    auto strongHandler = handler.toStrongRef();
    if (strongHandler)
    {
        QMetaObject::invokeMethod(strongHandler.data(), method);
        return true;
    }
    qWarning() << QString("Method %1 could not be invoked! Invalid QWeakPointer forwarded.")
                  .arg(QString(method));
    return false;
}

template<typename T>
static bool invokeMethod(QWeakPointer<T> handler,
                         const char *method,
                         QByteArray *message)
{
    auto strongHandler = handler.toStrongRef();
    if (strongHandler)
    {
        QMetaObject::invokeMethod(strongHandler.data(),
                                  method,
                                  Q_ARG(QByteArray, *message));
        return true;
    }
    qWarning() << QString("Method %1 could not be invoked! Invalid QWeakPointer forwarded.")
                  .arg(QString(method));
    return false;
}

//==================================================================================================
// Process
//==================================================================================================

DIALOGProcess &DIALOGProcess::GetInstance()
{
    static DIALOGProcess instance;
    return instance;
}

void DIALOGProcess::start(QThread::Priority priority)
{
    deleteLater(server);

    server = new Server(name,
                        Custom,
                        controlAddress.hostName,
                        controlAddress.port,
                        sender,
                        receiver);
    server->start(priority);
}

bool DIALOGProcess::started() const
{
    return bool(server);
}

void DIALOGProcess::stop()
{
    if (!started())
    {
        qWarning() << QString("Cannot stop process that has never been started!");
        return;
    }
    server->stop();
}

void DIALOGProcess::setControlServerAddress(const QString &address, quint16 port)
{
    controlAddress = ProcessAddress(address, port);
    if (server)
    {
        server->setControlServerAddress(address, port);
    }
}

bool DIALOGProcess::connectedToControlServer() const
{
    return started() && server->isConnectedToControlServer();
}

bool DIALOGProcess::setName(const QString &nameInit)
{
    if (started())
    {
        qWarning() << QString("The name of running server cannot be updated!");
        return false;
    }
    name = nameInit;
    return true;
}

QString DIALOGProcess::getName() const
{
    return name;
}

QPointer<Server> DIALOGProcess::getServer() const
{
    return server;
}

bool DIALOGProcess::tryRegisterCommand(QWeakPointer<DIALOGCommandHandler> command)
{
    return tryRegisterTypeImpl<DIALOGCommandHandler>(command,
                                                     DIALOGMessageHandlerType::CommandHandler,
                                                     "registerCommandSlot");
}

QSharedPointer<DIALOGCommandHandler> DIALOGProcess::tryRegisterCommand(const QString& name)
{
    return tryRegisterTypeImpl<DIALOGCommandHandler>(name,
                                                     DIALOGMessageHandlerType::CommandHandler,
                                                     "registerCommandSlot");
}

bool DIALOGProcess::tryRegisterProcedure(QWeakPointer<DIALOGProcedureProvider> procedure)
{
    return tryRegisterTypeImpl<DIALOGProcedureProvider>(procedure,
                                                        DIALOGMessageHandlerType::ProcedureProvider,
                                                        "registerProcedureSlot");
}

QSharedPointer<DIALOGProcedureProvider> DIALOGProcess::tryRegisterProcedure(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGProcedureProvider>(name,
                                                        DIALOGMessageHandlerType::ProcedureProvider,
                                                        "registerProcedureSlot");
}

bool DIALOGProcess::tryRegisterService(QWeakPointer<DIALOGServicePublisher> provider)
{
    return tryRegisterTypeImpl<DIALOGServicePublisher>(provider,
                                                       DIALOGMessageHandlerType::ServicePublisher,
                                                       "registerServiceSlot");
}

QSharedPointer<DIALOGServicePublisher> DIALOGProcess::tryRegisterService(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGServicePublisher>(name,
                                                       DIALOGMessageHandlerType::ServicePublisher,
                                                       "registerServiceSlot");
}

bool DIALOGProcess::tryRequestService(QWeakPointer<DIALOGServiceSubscriber> subscriber)
{
    return tryRegisterTypeImpl<DIALOGServiceSubscriber>(subscriber,
                                                        DIALOGMessageHandlerType::ServiceSubscriber,
                                                        "requestServiceSlot");
}

QSharedPointer<DIALOGServiceSubscriber> DIALOGProcess::tryRequestService(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGServiceSubscriber>(name,
                                                        DIALOGMessageHandlerType::ServiceSubscriber,
                                                        "requestServiceSlot");
}

bool DIALOGProcess::tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller)
{
    if (canContactControlServer())
    {
        int id = receiver->tryRegisterProcedureCaller(caller);
        if (id != -1)
        {
            auto strongCaller = caller.toStrongRef();
            if (strongCaller) {
                sender->callProcedureSlot(strongCaller->getName(),
                                          strongCaller->getParams(),
                                          id);
                return true;
            }
        }
    }
    return false;
}

QSharedPointer<DIALOGProcedureCaller> DIALOGProcess::tryCallProcedure(const QString &name,
                                                                      const QByteArray &params)
{
    auto caller = QSharedPointer<DIALOGProcedureCaller>(new DIALOGProcedureCaller(name, params));
    if (tryCallProcedure(caller.toWeakRef())){
        return caller;
    }
    return nullptr;
}

bool DIALOGProcess::tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller,
                                     const QString &processName)
{
    if (canContactControlServer())
    {
        int id = receiver->tryRegisterProcedureCaller(caller);
        if (id != -1)
        {
            auto strongCaller = caller.toStrongRef();
            if (strongCaller) {
                sender->callProcedureNameSlot(strongCaller->getName(),
                                              strongCaller->getParams(),
                                              processName,
                                              id);
                return true;
            }
        }
    }
    return false;
}

QSharedPointer<DIALOGProcedureCaller> DIALOGProcess::tryCallProcedure(const QString &name,
                                                                      const QByteArray &params,
                                                                      const QString &processName)
{
    auto caller = QSharedPointer<DIALOGProcedureCaller>(new DIALOGProcedureCaller(name, params));
    if (tryCallProcedure(caller.toWeakRef(), processName)){
        return caller;
    }
    return nullptr;
}

bool DIALOGProcess::tryCallProcedure(QWeakPointer<DIALOGProcedureCaller> caller,
                                     const QString &address,
                                     quint16 port)
{
    if (canContactControlServer())
    {
        int id = receiver->tryRegisterProcedureCaller(caller);
        if (id != -1)
        {
            auto strongCaller = caller.toStrongRef();
            if (strongCaller) {
                sender->callProcedureAddressSlot(strongCaller->getName(),
                                                 strongCaller->getParams(),
                                                 address,
                                                 port,
                                                 id);
                return true;
            }
        }
    }
    return false;
}

QSharedPointer<DIALOGProcedureCaller> DIALOGProcess::tryCallProcedure(const QString &name,
                                                                      const QByteArray &params,
                                                                      const QString &address,
                                                                      quint16 port)
{
    auto caller = QSharedPointer<DIALOGProcedureCaller>(new DIALOGProcedureCaller(name, params));
    if (tryCallProcedure(caller.toWeakRef(), address, port)){
        return caller;
    }
    return nullptr;
}

bool DIALOGProcess::trySendCommand(const QString &name, const QByteArray &message)
{
    if (canContactControlServer()) {
        sender->sendCommandSlot(name, message);
        return true;
    }
    return false;
}

bool DIALOGProcess::trySendCommand(const QString &name,
                                   const QByteArray &message,
                                   const QString &processName)
{
    if (canContactControlServer()) {
        sender->sendCommandNameSlot(name, message, processName);
        return true;
    }
    return false;
}

bool DIALOGProcess::trySendCommand(const QString &name,
                                   const QByteArray &message,
                                   const QString &address,
                                   quint16 port)
{
    if (canContactControlServer()) {
        sender->sendCommandAddressSlot(name, message, address, port);
        return true;
    }
    return false;
}

DIALOGProcess::DIALOGProcess()
    : name("Unnamed"),
      controlAddress(getenv("DIALOG_CONTROL_SERVER_ADDRESS"),
                     QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt())
{
    sender = new SenderThread();
    receiver = new ReceiverThread();
    receiver->setSenderThread(sender);
}

DIALOGProcess::~DIALOGProcess()
{
    deleteLater(server);
    deleteLater(receiver);
    deleteLater(sender);
}

bool DIALOGProcess::canContactControlServer()
{
    if (!started())
    {
        qWarning() << "Cannot contact Control Server before process start.";
        return false;
    }
    if (!server->waitForConnectionToControlServer(CONNECT_TIMEOUT))
    {
        qWarning() << "Could not send the message because Control server is not connected.";
        return false;
    }
    return true;
}

template<typename T>
bool DIALOGProcess::tryRegisterTypeImpl(QWeakPointer<T> handler,
                                        DIALOGMessageHandlerType type,
                                        const char* method)
{
    auto handlerName = tryGetName(handler);
    if (handlerName.isEmpty())
    {
        qWarning() << QString("%1 could not be registered! Invalid QWeakPointer forwarded.")
                      .arg(DIALOGMessageStrings[type]);
        return false;
    }
    bool success = sender->tryRegisterMessageSender(handler)
                   && receiver->tryRegisterMessageHandler(handler);
    if (success && started())
    {
        QMetaObject::invokeMethod(server, method, Q_ARG(QString, handlerName));
    }
    return success;
}

template<typename T>
QSharedPointer<T> DIALOGProcess::tryRegisterTypeImpl(const QString &name,
                                                     DIALOGMessageHandlerType type,
                                                     const char* method)
{
    QSharedPointer<T> handler = QSharedPointer<T>(new T(name), deleteLater);
    if (tryRegisterTypeImpl<T>(handler, type, method))
    {
        return handler;
    }
    return QSharedPointer<T>();
}

//==================================================================================================
// Nested Receiver and Sender threads
//==================================================================================================

DIALOGProcess::ReceiverThread::~ReceiverThread()
{
    DIALOGCommon::logMessage("Receiver destructor");
}

void DIALOGProcess::ReceiverThread::run()
{
    connect(server, &Server::controlServerConnectedSignal,
            this, &DIALOGProcess::ReceiverThread::controlServerConnectedSlot);
    connect(server, &Server::controlServerUnavailableSignal,
            this, &DIALOGProcess::ReceiverThread::controlServerUnavailableSlot);
    connect(server, &Server::lostServiceSubscriberSignal,
            this, &DIALOGProcess::ReceiverThread::lostServiceSubscriberSlot);
    connect(server, &Server::serviceUnavailableSignal,
            this, &DIALOGProcess::ReceiverThread::serviceUnavailableSlot);
    connect(server, &Server::newServiceSubscriberSignal,
            this, &DIALOGProcess::ReceiverThread::newServiceSubscriberSlot);
    connect(server, &Server::serviceRegistrationFailedSignal,
            this, &DIALOGProcess::ReceiverThread::serviceRegistrationFailedSlot);
    connect(server, &Server::serviceActivatedSignal,
            this, &DIALOGProcess::ReceiverThread::serviceActivatedSlot);
    connect(server, &Server::procedureUnavailableSignal,
            this, &DIALOGProcess::ReceiverThread::procedureUnavailableSlot);

    controlServerAvailable = true;
    virtualThreadEventLoop = new QEventLoop(this);
    virtualThreadEventLoop->exec();

    DIALOGCommon::logMessage("End of Receiver EventLoop");
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(
                                                         QWeakPointer<DIALOGCommandHandler> handler)
{
    return registerMessageHandlerImpl<DIALOGCommandHandler>(handler,
                                                            commands,
                                                          DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(
                                                      QWeakPointer<DIALOGServiceSubscriber> handler)
{
    return registerMessageHandlerImpl<DIALOGServiceSubscriber>(handler,
                                                               serviceSubscribers,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(
                                                       QWeakPointer<DIALOGServicePublisher> handler)
{
    return registerMessageHandlerImpl<DIALOGServicePublisher>(handler,
                                                               servicePublishers,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(
                                                      QWeakPointer<DIALOGProcedureProvider> handler)
{
    return registerMessageHandlerImpl<DIALOGProcedureProvider>(handler,
                                                         procedureProviders,
                                                       DIALOGMessageHandlerType::ProcedureProvider);
}

int DIALOGProcess::ReceiverThread::tryRegisterProcedureCaller(
                                                QWeakPointer<DIALOGProcedureCaller> procedureCaller)
{
    QString name = tryGetName<DIALOGProcedureCaller>(procedureCaller);
    if (!name.isEmpty())
    {
        callerId++;
        procedureCallers[name][callerId] = procedureCaller;
        allHandlers.append(procedureCaller);
        connect(procedureCaller.toStrongRef().data(), &QObject::destroyed,
                this, &ReceiverThread::objectDeletedSlot);
        return callerId;
    }
    return -1;
}

void DIALOGProcess::ReceiverThread::setSenderThread(QPointer<SenderThread> sender)
{
    senderThread = sender;
}

void DIALOGProcess::ReceiverThread::messageReceivedSlot(QString senderAddress,
                                                        quint16 senderPort,
                                                        QByteArray *header,
                                                        QByteArray *message)
{
    if (!header || !header->data() || header->size() == 0)
    {
        qWarning() << "Corrupted message received.";
        return;
    }
    QStringList headerList = QString(*header).split(SEPARATOR);
    if (headerList.size() == 0)
    {
        qWarning() << "Empty header received.";
        return;
    }
    const QString messageIdentifier = headerList.first();
    if (messageIdentifier == COMMAND_MESSAGE)
    {
        commandMassageReceived(headerList, message);
    }
    else if (messageIdentifier == SERVICE_MESSAGE)
    {
        serviceMessageReceived(headerList, message);
    }
    else if (messageIdentifier == PROCEDURE_MESSAGE)
    {
        procedureMessageReceived(headerList, message, senderAddress, senderPort);
    }
    delete header;
    delete message;
}

void DIALOGProcess::ReceiverThread::serverErrorSlot(QString)
{
    // All errors should be handled properly
}

void DIALOGProcess::ReceiverThread::controlServerConnectedSlot()
{
    if (!controlServerAvailable)
    {
        controlServerAvailable = true;
        for (auto handler : allHandlers)
        {
            invokeMethod(handler, "controlServerConnectedSlot");
        }
    }
}

void DIALOGProcess::ReceiverThread::controlServerUnavailableSlot()
{
    if (controlServerAvailable)
    {
        controlServerAvailable = false;
        for (auto handler : allHandlers)
        {
            invokeMethod(handler, "controlServerUnavailableErrorSlot");
        }
    }
}

void DIALOGProcess::ReceiverThread::lostServiceSubscriberSlot(const QString &name)
{
    if (servicePublishers.contains(name))
    {
        invokeMethod<DIALOGServicePublisher>(servicePublishers[name], "subscriberLostSlot");
    }
}

void DIALOGProcess::ReceiverThread::serviceUnavailableSlot(const QString& name)
{
    if (serviceSubscribers.contains(name))
    {
        invokeMethod<DIALOGServiceSubscriber>(serviceSubscribers[name],
                                              "serviceUnavailableErrorSlot");
    }
}

void DIALOGProcess::ReceiverThread::procedureUnavailableSlot(const QString &name, int callerId)
{
    if (procedureCallers.contains(name) && procedureCallers[name].contains(callerId))
    {
        invokeMethod<DIALOGProcedureCaller>(procedureCallers[name][callerId],
                                            "procedureUnavailableErrorSlot");
    }
}

void DIALOGProcess::ReceiverThread::newServiceSubscriberSlot(const QString &name)
{
    if (servicePublishers.contains(name))
    {
        invokeMethod<DIALOGServicePublisher>(servicePublishers[name], "subscriberConnectedSlot");
    }
}

void DIALOGProcess::ReceiverThread::serviceActivatedSlot(const QString &name)
{
    if (serviceSubscribers.contains(name))
    {
        invokeMethod<DIALOGServiceSubscriber>(serviceSubscribers[name],
                                              "serviceActivatedSlot");
    }
}

void DIALOGProcess::ReceiverThread::serviceRegistrationFailedSlot(const QString &name)
{
    if (servicePublishers.contains(name))
    {
        invokeMethod<DIALOGServicePublisher>(servicePublishers[name],
                                              "registrationFailedSlot");
    }
}

void DIALOGProcess::ReceiverThread::objectDeletedSlot()
{
    allHandlers.erase(std::remove_if(allHandlers.begin(),
                                     allHandlers.end(),
                                     [](QWeakPointer<DIALOGNamedData> value)
                                       {
                                           return !value.toStrongRef();
                                       }),
                      allHandlers.end());

    removeInvalidPointers<DIALOGCommandHandler>(commands, "unRegisterCommandSlot");
    removeInvalidPointers<DIALOGServiceSubscriber>(serviceSubscribers, "unSubscribeServiceSlot");
    removeInvalidPointers<DIALOGServicePublisher>(servicePublishers, "unRegisterServiceSlot");
    removeInvalidPointers<DIALOGProcedureProvider>(procedureProviders, "unRegisterProcedureSlot");

    removeInvalidProcedureCallers();
}

void DIALOGProcess::ReceiverThread::removeInvalidProcedureCallers()
{
    for (auto sameNameCallers : procedureCallers.values())
    {
        for (auto it = sameNameCallers.begin(); it != sameNameCallers.end();)
        {
            if (!it.value().toStrongRef())
            {
                it = sameNameCallers.erase(it);
                continue;
            }
            ++it;
        }
    }
    for (auto it = procedureCallers.begin(); it != procedureCallers.end();)
    {
        if (procedureCallers.empty())
        {
            it = procedureCallers.erase(it);
            continue;
        }
        ++it;
    }
}

bool DIALOGProcess::ReceiverThread::trySetResponseAddress(const QString &senderAddress,
                                                                int senderPort,
                                                                int callerId)
{
    if (senderThread)
    {
        senderThread->setProcedureResponseAddress(messageId, callerId, senderAddress, senderPort);
        return true;
    }
    return false;
}

void DIALOGProcess::ReceiverThread::commandMassageReceived(const QStringList &headerList,
                                                           QByteArray *message)
{
    QString commandName = headerList[1];
    if (commands.contains(commandName))
    {
        invokeMethod<DIALOGCommandHandler>(commands[commandName], "commandReceivedSlot", message);
    }
    else
    {
        qWarning() << QString("Unknown Command %1 received.").arg(commandName);
    }
}

void DIALOGProcess::ReceiverThread::serviceMessageReceived(const QStringList &headerList,
                                                           QByteArray *message)
{
    QString serviceName = headerList[1];
    if (serviceSubscribers.contains(serviceName))
    {
        auto subscriber = serviceSubscribers[serviceName];
        if (headerList.size()==2)
        {
           invokeMethod<DIALOGServiceSubscriber>(subscriber, "dataUpdatedSlot", message);
           return;
        }
        else if (headerList[3] == PUBLISHER_MESSAGE)
        {
           invokeMethod<DIALOGServiceSubscriber>(subscriber,"messageReceivedSlot", message);
           return;
        }
    }
    else if (servicePublishers.contains(serviceName) && headerList.size() > 2)
    {
        auto publisher = servicePublishers[serviceName];
        if (headerList[3] == SUBSCRIBER_MESSAGE)
        {
            invokeMethod<DIALOGServicePublisher>(publisher, "messageReceivedSlot", message);
            return;
        }
        else if (headerList[3] == UPDATE_REQUEST)
        {
            invokeMethod<DIALOGServicePublisher>(publisher,"dataUpdateRequestedSlot");
            return;
        }
    }
    qWarning() << QString("Unknown Service %1 received.").arg(serviceName);
}

void DIALOGProcess::ReceiverThread::procedureMessageReceived(const QStringList &headerList,
                                                             QByteArray *message,
                                                             QString senderAddress,
                                                             quint16 senderPort)
{
    QString procedureName = headerList[2];
    int callerId = headerList[3].toInt();
    if (headerList.size() == 6) {
        senderAddress = headerList[4];
        senderPort = headerList[5].toInt();
    }
    const QString messageIdentifier = headerList[1];
    if (messageIdentifier == PROCEDURE_CALL)
    {
        if(procedureProviders.contains(procedureName)
            && trySetResponseAddress(senderAddress, senderPort, callerId))
        {
            QMetaObject::invokeMethod(procedureProviders[procedureName].toStrongRef().data(),
                                      "callRequestedSlot",
                                      Q_ARG(QByteArray, *message),
                                      Q_ARG(int, messageId));
            messageId++;
        }
        else
        {
            server->procedureUnavailableSlot(procedureName,
                                             senderAddress,
                                             senderPort,
                                             callerId);
            qWarning() << QString("Call of unknown Procedure %1 received.").arg(procedureName);
        }
    }
    else if (procedureCallers.contains(procedureName)
             && procedureCallers[procedureName].contains(callerId))
    {
        auto caller = procedureCallers[procedureName][callerId];
        if (messageIdentifier == PROCEDURE_DATA)
        {
            invokeMethod<DIALOGProcedureCaller>(caller, "responseReceivedSlot", message);
        }
        else if (messageIdentifier == PROCEDURE_FAILED)
        {
            invokeMethod<DIALOGProcedureCaller>(caller, "callFailedErrorSlot", message);
        }
        else if (messageIdentifier == PROCEDURE_INVALID)
        {
            invokeMethod<DIALOGProcedureCaller>(caller, "paramsInvalidErrorSlot");
        }
        else
        {
            qWarning() << QString("Unknown Procedure message identifier %1 received.")
                          .arg(messageIdentifier);
        }
    }
    else
    {
        qWarning() << QString("Unknown Procedure %1 message %2 received.")
                      .arg(procedureName)
                      .arg(messageIdentifier);
    }
}

template<typename T>
bool DIALOGProcess::ReceiverThread::registerMessageHandlerImpl(QWeakPointer<T> handler,
                                                               QMap<QString, QWeakPointer<T>>& map,
                                                               DIALOGMessageHandlerType type)
{
    QString name = tryGetName<T>(handler);
    if (name.isEmpty())
    {
        return false;
    }
    if (!map.contains(name))
    {
        map[name] = handler;
        allHandlers.append(handler);
        connect(handler.toStrongRef().data(), &QObject::destroyed,
                this, &ReceiverThread::objectDeletedSlot);
        return true;
    }
    qWarning() << QString("%1 has been already registered!")
                  .arg(DIALOGMessageStrings[type]);
    return false;
}

template<typename T>
void DIALOGProcess::ReceiverThread::removeInvalidPointers(QMap<QString, QWeakPointer<T> > &map,
                                                          const char *unregisterMethod)
{
    for (auto it = map.begin(); it != map.end();)
    {
        if (!it.value().toStrongRef())
        {
            QMetaObject::invokeMethod(server,
                                      unregisterMethod,
                                      Q_ARG(const QString&, it.key()));
            it = map.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

DIALOGProcess::SenderThread::~SenderThread()
{
    DIALOGCommon::logMessage("Sender destructor");
}

void DIALOGProcess::SenderThread::run()
{
    server->connectToControlServerSlot();

    for (const auto name : commandsToRegister)
    {
        server->registerCommandSlot(name);
    }
    for (const auto name : servicesToRegister)
    {
        server->registerServiceSlot(name);
    }
    for (const auto name : servicesToRequest)
    {
        server->requestServiceSlot(name);
    }
    for (const auto name : proceduresToRegister)
    {
        server->registerProcedureSlot(name);
    }

    virtualThreadEventLoop = new QEventLoop(this);
    virtualThreadEventLoop->exec();

    DIALOGCommon::logMessage("End of Sender EventLoop");
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(
                                                          QWeakPointer<DIALOGCommandHandler> sender)
{
    return registerMessageSenderImpl<DIALOGCommandHandler>(
                                                         sender,
                                                         commandsToRegister,
                                                         DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(
                                                       QWeakPointer<DIALOGServiceSubscriber> sender)
{
    if(registerMessageSenderImpl<DIALOGServiceSubscriber>(
                                                       sender,
                                                       servicesToRequest,
                                                       DIALOGMessageHandlerType::ServiceSubscriber))
    {
        auto rawSender = sender.toStrongRef().data();
        connect(rawSender, &DIALOGServiceSubscriber::requestDataUpdateSignal,
                this, &SenderThread::requestServiceUpdateSlot);
        connect(rawSender, &DIALOGServiceSubscriber::sendMessageToProviderSignal,
                this, &SenderThread::sendServiceSubscriberMessageSlot);
        return true;
    }
    return false;
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(
                                                        QWeakPointer<DIALOGServicePublisher> sender)
{
    if (registerMessageSenderImpl<DIALOGServicePublisher>(
                                                     sender,
                                                     servicesToRegister,
                                                     DIALOGMessageHandlerType::ServicePublisher))
    {
        auto rawSender = sender.toStrongRef().data();
        connect(rawSender, &DIALOGServicePublisher::dataUpdatedSignal,
                this, &SenderThread::sendServiceDataSlot);
        connect(rawSender, &DIALOGServicePublisher::sendMessageToSubscribersSignal,
                this, &SenderThread::sendServicePublisherMessageSlot);
        return true;
    }
    return false;
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(
                                                       QWeakPointer<DIALOGProcedureProvider> sender)
{
    if (registerMessageSenderImpl<DIALOGProcedureProvider>(
                                                     sender,
                                                     proceduresToRegister,
                                                     DIALOGMessageHandlerType::ProcedureProvider))
    {
        connect(sender.toStrongRef().data(), &DIALOGProcedureProvider::callFinishedSignal,
                this, &SenderThread::sendProcedureReturnValueSlot);
        return true;
    }
    return false;
}

void DIALOGProcess::SenderThread::setProcedureResponseAddress(int callId,
                                                              int callerId,
                                                              const QString& address,
                                                              quint16 port)
{
    if (!responseAddresses.contains(callId))
    {
        responseAddresses[callId] = ResponseAddress(address, port, callerId);
        return;
    }
    qWarning() << QString("Response address for messageId %1 has been already set.").arg(callId);
}

void DIALOGProcess::SenderThread::sendCommandSlot(const QString& name, const QByteArray& message)
{
    emit sendCommandMessageSignal(name, new QByteArray(message));
}

void DIALOGProcess::SenderThread::sendCommandNameSlot(const QString& name,
                                                      const QByteArray& message,
                                                      const QString& processName)
{
    emit sendCommandNameMessageSignal(name, new QByteArray(message), processName);
}

void DIALOGProcess::SenderThread::sendCommandAddressSlot(const QString &name,
                                                         const QByteArray &message,
                                                         const QString &address,
                                                         quint16 port)
{
    emit sendCommandAddressMessageSignal(name, new QByteArray(message), address, port);
}

void DIALOGProcess::SenderThread::sendServiceDataSlot(const QByteArray& data)
{
    QString senderName = getSenderName<DIALOGServicePublisher>(sender());
    if(!senderName.isEmpty())
    {
        emit sendServiceMessageSignal(senderName, new QByteArray(data));
    }
}

void DIALOGProcess::SenderThread::sendServicePublisherMessageSlot(const QByteArray &message)
{
    QString senderName = getSenderName<DIALOGServicePublisher>(sender());
    if(!senderName.isEmpty())
    {
        emit sendServicePublisherMessageSignal(senderName, new QByteArray(message));
    }
}

void DIALOGProcess::SenderThread::requestServiceUpdateSlot()
{
    QString senderName = getSenderName<DIALOGServiceSubscriber>(sender());
    if(!senderName.isEmpty())
    {
        emit sendServiceDataRequestSignal(senderName);
    }
}

void DIALOGProcess::SenderThread::sendServiceSubscriberMessageSlot(const QByteArray &message)
{
    QString senderName = getSenderName<DIALOGServiceSubscriber>(sender());
    if(!senderName.isEmpty())
    {
        emit sendServiceSubscriberMessageSignal(senderName, new QByteArray(message));
    }
}

void DIALOGProcess::SenderThread::callProcedureSlot(const QString &name,
                                                    const QByteArray &message,
                                                    int callerId)
{
    emit sendProcedureCallMessageSignal(name, new QByteArray(message), callerId);
}

void DIALOGProcess::SenderThread::callProcedureNameSlot(const QString &name,
                                                        const QByteArray &message,
                                                        const QString &processName,
                                                        int callerId)
{
    emit sendProcedureCallNameMessageSignal(name, new QByteArray(message), processName, callerId);
}

void DIALOGProcess::SenderThread::callProcedureAddressSlot(const QString &name,
                                                           const QByteArray &message,
                                                           const QString &address,
                                                           quint16 port,
                                                           int callerId)
{
    emit sendProcedureCallAddressMessageSignal(name,
                                               new QByteArray(message),
                                               address,
                                               port,
                                               callerId);
}

void DIALOGProcess::SenderThread::sendProcedureReturnValueSlot(const QByteArray &data,
                                                               int callId)
{
    QString senderName = getSenderName<DIALOGProcedureProvider>(sender());
    if (!senderName.isEmpty() && responseAddresses.contains(callId))
    {
        ResponseAddress all = responseAddresses[callId];
        emit sendProcedureReturnMessageSignal(senderName,
                                              new QByteArray(data),
                                              all.address,
                                              all.port,
                                              all.objectId);
        return;
    }
    qWarning() << "Could not deliver response to procedure call. Caller address is unknown.";
}

void DIALOGProcess::SenderThread::sendProcedureFailedSlot(const QByteArray &message, int callId)
{
    QString senderName = getSenderName<DIALOGProcedureProvider>(sender());
    if (!senderName.isEmpty() && responseAddresses.contains(callId))
    {
        ResponseAddress all = responseAddresses[callId];
        emit sendProcedureFailedMessageSignal(senderName,
                                              new QByteArray(message),
                                              all.address,
                                              all.port,
                                              all.objectId);
        return;
    }
    qWarning() << "Could not deliver response to procedure call. Caller address is unknown.";
}

void DIALOGProcess::SenderThread::sendParamsInvalidErrorSlot(int callId)
{
    QString senderName = getSenderName<DIALOGProcedureProvider>(sender());
    if (!senderName.isEmpty() && responseAddresses.contains(callId))
    {
        ResponseAddress all = responseAddresses[callId];
        emit sendProcedureInvalidParamsMessageSignal(senderName,
                                                     all.address,
                                                     all.port,
                                                     all.objectId);
        return;
    }
    qWarning() << "Could not deliver response to procedure call. Caller address is unknown.";
}

template<typename T>
bool DIALOGProcess::SenderThread::registerMessageSenderImpl(
                                                QWeakPointer<T> sender,
                                                QStringList &list,
                                                DIALOGMessageHandlerType type)
{
    QString name = tryGetName(sender);
    if (name.isEmpty())
    {
        return false;
    }
    if (list.contains(name))
    {
        qWarning() << QString("%1 has been already registered!")
                      .arg(DIALOGMessageStrings[type]);
        return false;
    }
    list.append(name);
    return true;
}

template<typename T>
QString DIALOGProcess::SenderThread::getSenderName(QObject *obj)
{
    if(obj)
    {
        T* sender = qobject_cast<T*>(obj);
        if (sender) {
            return sender->getName();
        }
    }
    qWarning() << "Slot was triggered by invalid sender. His name could not be extracted.";
    return nullptr;
}

//==================================================================================================
// Base data class
//==================================================================================================

DIALOGNamedData::DIALOGNamedData(const QString &name, QObject *parent)
    : QObject(parent),
      name(name),
      error(DIALOGErrorCode::NoError)
{}

void DIALOGNamedData::setData(const QByteArray &dataInit)
{
    QMutexLocker locker(&mutex);
    data = dataInit;
}

void DIALOGNamedData::setError(DIALOGErrorCode errorInit)
{
    QMutexLocker locker(&mutex);
    error = errorInit;
}

QByteArray DIALOGNamedData::getData() const
{
    QMutexLocker locker(&mutex);
    return data;
}

DIALOGErrorCode DIALOGNamedData::getError() const
{
    QMutexLocker locker(&mutex);
    return error;
}

bool DIALOGNamedData::errorOccurred() const
{
    return error != DIALOGErrorCode::NoError;
}

QString DIALOGNamedData::getName() const
{
    QMutexLocker locker(&mutex);
    return name;
}

void DIALOGNamedData::controlServerConnectedSlot()
{
    emit controlServerConnectedSignal();
}

void DIALOGNamedData::controlServerUnavailableErrorSlot()
{
    setError(DIALOGErrorCode::LostControlServer);
    emit controlServerUnavailableErrorSignal();
}

//==================================================================================================
// Command
//==================================================================================================

DIALOGCommandHandler::DIALOGCommandHandler(const QString &name, QObject *parent)
    : DIALOGNamedData(name, parent)
{}

void DIALOGCommandHandler::commandReceivedSlot(const QByteArray &message)
{
    setData(message);
    emit commandReceivedSignal(message);
}

//==================================================================================================
// Procedure
//==================================================================================================

DIALOGProcedureProvider::DIALOGProcedureProvider(const QString& nameInit, QObject *parent)
    : DIALOGNamedData(nameInit, parent)
{}

void DIALOGProcedureProvider::callFinishedSlot(const QByteArray &response, int callId)
{
    setData(response);
    emit callFinishedSignal(response, callId);
}

void DIALOGProcedureProvider::callFailedErrorSlot(const QByteArray &message, int callId)
{
    setError(DIALOGErrorCode::CallFailed);
    emit callFailedErrorSignal(message, callId);
}

void DIALOGProcedureProvider::paramsInvalidErrorSlot(int callId)
{
    setError(DIALOGErrorCode::InvalidParams);
    emit paramsInvalidErrorSignal(callId);
}

void DIALOGProcedureProvider::callRequestedSlot(const QByteArray &params, int callId)
{
    emit callRequestedSignal(params, callId);
}

DIALOGProcedureCaller::DIALOGProcedureCaller(const QString &name,
                                             const QByteArray &paramsInit,
                                             QObject *parent)
    : DIALOGNamedData(name, parent),
      params(paramsInit)
{}

QByteArray DIALOGProcedureCaller::waitForResponse(bool &ok, int sTimeout)
{
    if (responseReceived())
    {
        return getData();
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &DIALOGProcedureCaller::responseReceivedSignal,
            &loop, &QEventLoop::quit);
    connect(this, &DIALOGProcedureCaller::callFailedErrorSignal,
            &loop, &QEventLoop::quit);
    connect(this, &DIALOGProcedureCaller::procedureUnavailableErrorSignal,
            &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout,
            &loop, &QEventLoop::quit);
    timer.start(sTimeout * 1000);
    loop.exec();

    ok = responseReceived();
    return getData();
}

bool DIALOGProcedureCaller::responseReceived() const
{
    QMutexLocker locker(&mutex);
    return hasResponse;
}

QByteArray DIALOGProcedureCaller::getParams() const
{
    QMutexLocker locker(&mutex);
    return params;
}

void DIALOGProcedureCaller::waitForTimeout(int sTimeout)
{
    QTimer::singleShot(sTimeout * 1000,
                       this,
                       &DIALOGProcedureCaller::timeoutPassedErrorSlot);
}

void DIALOGProcedureCaller::responseReceivedSlot(const QByteArray& response)
{
    setData(response);
    setHasResponse(true);

    emit responseReceivedSignal(response);
}

void DIALOGProcedureCaller::callFailedErrorSlot(const QByteArray &message)
{
    setError(DIALOGErrorCode::CallFailed);
    emit callFailedErrorSignal(message);
}

void DIALOGProcedureCaller::procedureUnavailableErrorSlot()
{
    setError(DIALOGErrorCode::Unavailable);
    emit procedureUnavailableErrorSignal();
}

void DIALOGProcedureCaller::paramsInvalidErrorSlot()
{
    setError(DIALOGErrorCode::InvalidParams);
    emit paramsInvalidErrorSignal();
}

void DIALOGProcedureCaller::timeoutPassedErrorSlot()
{
    if(!responseReceived())
    {
        setError(DIALOGErrorCode::Timeout);
        emit timeoutPassedErrorSignal();
    }
}

void DIALOGProcedureCaller::setHasResponse(bool response)
{
    QMutexLocker locker(&mutex);
    hasResponse = response;
}

//==================================================================================================
// Service
//==================================================================================================

DIALOGServicePublisher::DIALOGServicePublisher(const QString &name, QObject *parent)
    : DIALOGNamedData(name, parent)
{}

bool DIALOGServicePublisher::hasSubscriber() const
{
    return getSubscriberCount() > 0;
}

int DIALOGServicePublisher::getSubscriberCount() const
{
    QMutexLocker locker(&mutex);
    return subscriberCount;
}

void DIALOGServicePublisher::updateDataSlot(const QByteArray &data)
{
    setData(data);
    emit dataUpdatedSignal(getData());
}

void DIALOGServicePublisher::sendMessageToSubscribersSlot(const QByteArray &message)
{
    emit sendMessageToSubscribersSignal(message);
}

void DIALOGServicePublisher::dataUpdateRequestedSlot()
{
    emit dataUpdateRequestedSignal();
}

void DIALOGServicePublisher::messageReceivedSlot(const QByteArray &message)
{
    emit messageReceivedSignal(message);
}

void DIALOGServicePublisher::subscriberConnectedSlot()
{
    QMutexLocker locker(&mutex);
    subscriberCount++;
    locker.unlock(); // Needs to be there connect to signal is like a call in direct case...
    emit subscriberConnectedSignal();
}

void DIALOGServicePublisher::subscriberLostSlot()
{
    QMutexLocker locker(&mutex);
    subscriberCount--;
    locker.unlock(); // Needs to be there connect to signal is like a call in direct case...
    emit subscriberLostSignal();
}

void DIALOGServicePublisher::registrationFailedSlot()
{
    setError(DIALOGErrorCode::RegistrationFailed);
    emit registrationFailedSignal();
}

DIALOGServiceSubscriber::DIALOGServiceSubscriber(const QString &name, QObject *parent)
    : DIALOGNamedData(name, parent)
{ }

void DIALOGServiceSubscriber::sendMessageToProviderSlot(const QByteArray &message)
{
    emit sendMessageToProviderSignal(message);
}

void DIALOGServiceSubscriber::requestDataUpdateSlot()
{
    emit requestDataUpdateSignal();
}

void DIALOGServiceSubscriber::dataUpdatedSlot(const QByteArray &data)
{
    setData(data);
    emit dataUpdatedSignal(data);
}

void DIALOGServiceSubscriber::messageReceivedSlot(const QByteArray &message)
{
    emit messageReceivedSignal(message);
}

void DIALOGServiceSubscriber::serviceUnavailableErrorSlot()
{
    setError(DIALOGErrorCode::Unavailable);
    emit serviceUnavailableErrorSignal();
}

void DIALOGServiceSubscriber::serviceActivatedSlot()
{
    emit serviceActivatedSignal();
}
