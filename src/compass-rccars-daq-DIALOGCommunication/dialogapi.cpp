#include "dialogapi.h"

static void deleteLater(QObject *obj)
{
    if (obj) {
        obj->deleteLater();
    }
}

template<typename T>
static QString tryGetName(QWeakPointer<T> handler)
{
    auto strongHandler = handler.toStrongRef();
    if (!strongHandler)
    {
        qWarning() << QString("Handler name could not be extracted! Invalid QWeakPointer forwarded.");
        return "";
    }
    return strongHandler->getName();
}

DIALOGProcess &DIALOGProcess::GetInstance()
{
    static DIALOGProcess instance;
    return instance;
}

DIALOGProcess::DIALOGProcess()
    : name("Unnamed"),
      controlAddress(getenv("DIALOG_CONTROL_SERVER_ADDRESS"),
                     QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt())
{
    sender = new SenderThread();
    receiver = new ReceiverThread();
}

DIALOGProcess::~DIALOGProcess()
{
    deleteLater(server);
    deleteLater(receiver);
    deleteLater(sender);
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

void DIALOGProcess::stop()
{
    server->stop();
}

void DIALOGProcess::setControlServerAddress(const QString &address, quint16 port)
{
    controlAddress = ProcessAddress(address, port);
    if (server) {
        server->setControlServerAddress(address, port);
    }
}

bool DIALOGProcess::setName(const QString &nameInit)
{
    if (server) {
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
                                                       DIALOGMessageHandlerType::CommandHandler);
}

QSharedPointer<DIALOGCommandHandler> DIALOGProcess::tryRegisterCommand(const QString& name)
{
    return tryRegisterTypeImpl<DIALOGCommandHandler>(name,
                                                       DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::tryRegisterProcedure(QWeakPointer<DIALOGProcedureProvider> procedure)
{
    return tryRegisterTypeImpl<DIALOGProcedureProvider>(procedure,
                                                    DIALOGMessageHandlerType::ProcedureProvider);
}

QSharedPointer<DIALOGProcedureProvider> DIALOGProcess::tryRegisterProcedure(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGProcedureProvider>(name,
                                                    DIALOGMessageHandlerType::ProcedureProvider);
}

bool DIALOGProcess::tryRegisterService(QWeakPointer<DIALOGServicePublisher> provider)
{
    return tryRegisterTypeImpl<DIALOGServicePublisher>(provider,
                                                         DIALOGMessageHandlerType::ServicePublisher);
}

QSharedPointer<DIALOGServicePublisher> DIALOGProcess::tryRegisterService(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGServicePublisher>(name,
                                                        DIALOGMessageHandlerType::ServicePublisher);
}

bool DIALOGProcess::tryRequestService(QWeakPointer<DIALOGServiceSubscriber> subscriber)
{
    return tryRegisterTypeImpl<DIALOGServiceSubscriber>(subscriber,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

QSharedPointer<DIALOGServiceSubscriber> DIALOGProcess::tryRequestService(const QString &name)
{
    return tryRegisterTypeImpl<DIALOGServiceSubscriber>(name,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

QSharedPointer<DIALOGProcedureCaller> DIALOGProcess::tryCallProcedure(const QString &name,
                                                                   const QByteArray &params)
{
    if (server && server->waitForConnectionToControlServer()) {
        QSharedPointer<DIALOGProcedureCaller> caller =
                QSharedPointer<DIALOGProcedureCaller>(new DIALOGProcedureCaller(name));
        receiver->tryRegisterMessageHandler(caller.toWeakRef());
        sender->callProcedureSlot(name, params);
        return caller;
    }
    return nullptr;
}

bool DIALOGProcess::trySendCommand(const QString &name, const QByteArray &message)
{
    if (server->waitForConnectionToControlServer()) {
        QByteArray* commandMessage = new QByteArray();
        commandMessage->append(message);
        sender->sendCommandSlot(name, commandMessage);
    }
    return true;
}

bool DIALOGProcess::trySendDirectCommand(const QString &name,
                                      const QByteArray &message,
                                      const QString &processName)
{
    if (server && server->waitForConnectionToControlServer()) {
        QByteArray* commandMessage = new QByteArray();
        commandMessage->append(message);
        sender->sendDirectCommandNameSlot(name, commandMessage, processName);
    }
    return true;
}

bool DIALOGProcess::trySendDirectCommand(const QString &name, const QByteArray &message, const QString &address, quint16 port)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(message);
    sender->sendDirectCommandAddressSlot(name, commandMessage, address, port);
    return true;
}

DIALOGProcess::SenderThread::~SenderThread()
{
    DIALOGCommon::logMessage("Sender destructor");
}

void DIALOGProcess::SenderThread::run()
{
    server->connectToControlServerSlot();

    for (const auto name : commandsToRegister) {
        server->registerCommandSlot(name);
    }

    for (const auto name : servicesToRegister) {
        server->registerServiceSlot(name);
    }

    for (const auto name : servicesToRequest) {
        server->requestServiceSlot(name);
    }

    for (const auto name : proceduresToRegister) {
        server->registerProcedureSlot(name);
    }

    virtualThreadEventLoop = new QEventLoop(this);
    virtualThreadEventLoop->exec();

    DIALOGCommon::logMessage("End of Sender EventLoop");
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(QWeakPointer<DIALOGCommandHandler> sender)
{
    return registerMessageSenderImpl<DIALOGCommandHandler>(
                                                         sender,
                                                         commandsToRegister,
                                                         DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(QWeakPointer<DIALOGServiceSubscriber> sender)
{
    return registerMessageSenderImpl<DIALOGServiceSubscriber>(
                                                       sender,
                                                       servicesToRequest,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(QWeakPointer<DIALOGServicePublisher> sender)
{
    if (registerMessageSenderImpl<DIALOGServicePublisher>(
                                                     sender,
                                                     servicesToRegister,
                                                     DIALOGMessageHandlerType::ServicePublisher))
    {
        connect(sender.toStrongRef().data(), &DIALOGServicePublisher::dataUpdatedSignal,
                this, &SenderThread::sendServiceSlot);
        return true;
    }
    return false;
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(QWeakPointer<DIALOGProcedureCaller> sender)
{
    // Sender thread does not work with procedure callers yet.
    return true;
}

bool DIALOGProcess::SenderThread::tryRegisterMessageSender(QWeakPointer<DIALOGProcedureProvider> sender)
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

void DIALOGProcess::SenderThread::sendCommandSlot(QString name, QByteArray *message)
{
    emit sendCommandMessageSignal(name, message);
}

void DIALOGProcess::SenderThread::sendDirectCommandNameSlot(QString name,
                                                        QByteArray *message,
                                                        QString processName)
{
    emit sendDirectCommandNameMessageSignal(name, message, processName);
}

void DIALOGProcess::SenderThread::sendDirectCommandAddressSlot(QString name,
                                                               QByteArray *message,
                                                               QString address,
                                                               quint16 port)
{
    emit sendDirectCommandAddressMessageSignal(name, message, address, port);
}

void DIALOGProcess::SenderThread::sendServiceSlot(QString name, QByteArray message)
{
//    if (server->isConnectedToControlServer()) {
        QByteArray* serviceMessage = new QByteArray();
        serviceMessage->append(message);
        emit sendServiceMessageSignal(name, serviceMessage);
//    } else {
//        qDebug()<<"No connection to control server.";
//    }
}

void DIALOGProcess::SenderThread::callProcedureSlot(QString name, QByteArray message)
{
    if (server->isConnectedToControlServer()) {
        QByteArray* serviceMessage = new QByteArray();
        serviceMessage->append(message);
        emit sendProcedureCallMessageSignal(name, serviceMessage);
    }
}

void DIALOGProcess::SenderThread::sendProcedureReturnValueSlot(QString name,
                                                               QByteArray data,
                                                               QString address,
                                                               quint16 port)
{
    QByteArray* serviceMessage = new QByteArray();
    serviceMessage->append(data);
    emit sendProcedureReturnMessageSignal(name, serviceMessage, address, port);
}

DIALOGProcess::ReceiverThread::~ReceiverThread()
{
    DIALOGCommon::logMessage("Receiver destructor");
}

void DIALOGProcess::ReceiverThread::run()
{
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
    // Receiver does not work with service providers yet.
    return true;
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(
                                                        QWeakPointer<DIALOGProcedureCaller> handler)
{
    QString name = tryGetName<DIALOGProcedureCaller>(handler);
    if (name.isEmpty())
    {
        return false;
    }
//    Replaces original handler. Currently it is not possible to call a method more times at ones.
    procedureCallers[name] = handler;
    return true;
}

bool DIALOGProcess::ReceiverThread::tryRegisterMessageHandler(QWeakPointer<DIALOGProcedureProvider> handler)
{
    return registerMessageHandlerImpl<DIALOGProcedureProvider>(handler,
                                                               procedureProviders,
                                                       DIALOGMessageHandlerType::ProcedureProvider);
}

void DIALOGProcess::ReceiverThread::messageReceivedSlot(QString senderName,
                                                        quint16 senderPort,
                                                        QByteArray *header,
                                                        QByteArray *message)
{
    if (!header || !header->data() || header->size() == 0)
    {
        std::cerr << "Corrupted message received." << std::endl;
        return;
    }

    QList<QByteArray> headerList = header->split(SEPARATOR);
    if (headerList.size() == 0)
    {
        qWarning() << "Empty header received.";
        return;
    }

    if (headerList[0] == COMMAND_MESSAGE)
    {
        QString command = headerList[1];
        if (commands.contains(command)) {
            invokeMethod<DIALOGCommandHandler>(commands[command],
                                               "commandReceivedSlot",
                                               message);
        }
        else {
            qWarning() << "Unknown command arrived.";
        }
    }
    else if (headerList[0] == SERVICE_MESSAGE)
    {
        QString service = headerList[1];
        if (serviceSubscribers.contains(service)){
            invokeMethod<DIALOGServiceSubscriber>(serviceSubscribers[service],
                                                  "dataUpdatedSlot",
                                                  message);
        }
        else {
            qWarning() << "Unknown service arrived.";
        }
    }
    else if (headerList[0] == PROCEDURE_MESSAGE)
    {
        if (headerList[1] == PROCEDURE_CALL)
        {
            QString procedure = headerList[2];
            if (procedureProviders.contains(procedure))
            {
                QMetaObject::invokeMethod(procedureProviders[procedure].toStrongRef().data(),
                                          "callRequestedSlot",
                                          Q_ARG(QByteArray, *message),
                                          Q_ARG(QString, headerList[3]),
                                          Q_ARG(int, headerList[4].toInt()));
            }
            else
            {
                qWarning() << "Unknown procedure arrived.";
            }
        }
        else if (headerList[1] == PROCEDURE_DATA)
        {
            QString procedure = headerList[2];

            if (procedureCallers.contains(procedure))
            {
                invokeMethod<DIALOGProcedureCaller>(procedureCallers[procedure],
                                                    "setDataSlot",
                                                    message);
            }
            else
            {
                std::cerr << "Unknown procedure data arrived." << std::endl;
            }
        }
        else
        {
            std::cerr << "Unknown procedure message arrived." << std::endl;
        }
    }
    delete header;
    delete message;
}

DIALOGCommandHandler::DIALOGCommandHandler(const QString &nameInit, QObject *parent)
    : DIALOGNamedData(nameInit, parent)
{

}

void DIALOGCommandHandler::commandReceivedSlot(const QByteArray &message)
{
    emit commandReceivedSignal(message);
}

DIALOGServicePublisher::DIALOGServicePublisher(const QString &nameInit, QObject *parent)
    : DIALOGNamedData(nameInit, parent)
{

}

void DIALOGServicePublisher::updateDataSlot(const QByteArray &dataInit)
{
    data = dataInit;
    emit dataUpdatedSignal(name, data);
}

DIALOGServiceSubscriber::DIALOGServiceSubscriber(const QString &nameInit, QObject *parent)
    : DIALOGNamedData(nameInit, parent)
{

}

void DIALOGServiceSubscriber::dataUpdatedSlot(const QByteArray &dataInit)
{
    data = dataInit;
    emit dataUpdatedSignal(data);
}

DIALOGProcedureCaller::DIALOGProcedureCaller(const QString &nameInit,
                                             const QByteArray &paramsInit,
                                             QObject *parent)
    : DIALOGNamedData(nameInit, parent),
      params(paramsInit)
{

}

QByteArray DIALOGProcedureCaller::waitForResponse(bool &ok, int sTimeout)
{
    if (responseReceived) {
        return tryGetData();
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &DIALOGProcedureCaller::responseReceivedSignal, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    timer.start(sTimeout * 1000);
    loop.exec();

    ok = responseReceived;
    return data;
}

void DIALOGProcedureCaller::responseReceivedSlot(const QByteArray& response)
{
    mutex.lock();
    data = response;
    hasResponse = true;
    mutex.unlock();
    emit responseReceivedSignal(response);
}


DIALOGProcedureProvider::DIALOGProcedureProvider(const QString& nameInit, QObject *parent)
    : DIALOGNamedData(nameInit, parent)
{

}

void DIALOGProcedureProvider::callRequestedSlot(const QByteArray &params)
{
    emit callRequestedSignal(params);
}

void DIALOGProcedureProvider::callFinishedSlot(const QByteArray &response)
{
    data = response;
    emit callFinishedSignal(name, response, address, port);
}

template<typename T>
bool DIALOGProcess::tryRegisterTypeImpl(QWeakPointer<T> handler, DIALOGMessageHandlerType type)
{
    auto strongHandler = handler.toStrongRef();
    if (strongHandler)
    {
//        QString name = strongCommand->getName();
//        if (server)
//        {
//            QMetaObject::invokeMethod(server, "registerCommandSlot", Q_ARG(QString, name));
//        }

        if (sender->tryRegisterMessageSender(handler))
        {
            return receiver->tryRegisterMessageHandler(handler);
        }
        qWarning() << QString("%1 %2 has been already registered.")
                      .arg(DIALOGMessageStrings[type])
                      .arg(strongHandler->getName());
        return false;
    }
    qWarning() << QString("%1 could not be registered! Invalid QWeakPointer forwarded.")
                  .arg(DIALOGMessageStrings[type]);
    return false;
}

template<typename T>
QSharedPointer<T> DIALOGProcess::tryRegisterTypeImpl(const QString &name,
                                                       DIALOGMessageHandlerType type)
{
    QSharedPointer<T> handler = QSharedPointer<T>(new T(name), deleteLater);
    if (tryRegisterTypeImpl<T>(handler, type))
    {
        return handler;
    }
    return QSharedPointer<T>();
}

template<typename T>
bool DIALOGProcess::ReceiverThread::registerMessageHandlerImpl(
        QWeakPointer<T> handler,
        QMap<QString, QWeakPointer<T> >& map,
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
        return true;
    }
    qWarning() << QString("%1 has been already registered!")
                  .arg(DIALOGMessageStrings[type]);
    return false;
}

template<typename T>
bool DIALOGProcess::ReceiverThread::invokeMethod(QWeakPointer<T> handler,
                                                 const char *method,
                                                 QByteArray *message)
{
    auto strongHandler = handler.toStrongRef();
    if (strongHandler)
    {
        QMetaObject::invokeMethod(strongHandler.data(),
                                  method,
                                  Q_ARG( QByteArray, *message ) );
        return true;
    }
    qWarning() << "Handler is no longer valid.";
    return false;
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

DIALOGNamedData::DIALOGNamedData(const QString &nameInit, QObject *parent)
    : QObject(parent)
{
    name = nameInit;
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
    emit controlServerUnavailableErrorSignal();
}
