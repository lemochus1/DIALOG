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

bool DIALOGProcess::registerCommand(QWeakPointer<DIALOGCommandHandler> command)
{
    return handleMessageTypeImpl<DIALOGCommandHandler>(command,
                                                       DIALOGMessageHandlerType::CommandHandler);
}

QSharedPointer<DIALOGCommandHandler> DIALOGProcess::registerCommand(const QString& name)
{
    return handleMessageTypeImpl<DIALOGCommandHandler>(name,
                                                       DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::registerProcedure(QWeakPointer<DIALOGProcedurePublisher> procedure)
{
    return handleMessageTypeImpl<DIALOGProcedurePublisher>(procedure,
                                                    DIALOGMessageHandlerType::ProcedurePublisher);
}

QSharedPointer<DIALOGProcedurePublisher> DIALOGProcess::registerProcedure(const QString &name)
{
    return handleMessageTypeImpl<DIALOGProcedurePublisher>(name,
                                                    DIALOGMessageHandlerType::ProcedurePublisher);
}

bool DIALOGProcess::registerService(QWeakPointer<DIALOGServicePublisher> publisher)
{
    return handleMessageTypeImpl<DIALOGServicePublisher>(publisher,
                                                         DIALOGMessageHandlerType::ServicePublisher);
}

QSharedPointer<DIALOGServicePublisher> DIALOGProcess::registerService(const QString &name)
{
    return handleMessageTypeImpl<DIALOGServicePublisher>(name,
                                                         DIALOGMessageHandlerType::ServicePublisher);
}

bool DIALOGProcess::requestService(QWeakPointer<DIALOGServiceSubscriber> subscriber)
{
    return handleMessageTypeImpl<DIALOGServiceSubscriber>(subscriber,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

QSharedPointer<DIALOGServiceSubscriber> DIALOGProcess::requestService(const QString &name)
{
    return handleMessageTypeImpl<DIALOGServiceSubscriber>(name,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

QSharedPointer<DIALOGProcedureCaller> DIALOGProcess::callProcedure(const QString &name,
                                                                   const QByteArray &params)
{
    if (server && server->waitForConnectionToControlServer()) {
        QSharedPointer<DIALOGProcedureCaller> caller =
                QSharedPointer<DIALOGProcedureCaller>(new DIALOGProcedureCaller(name));
        receiver->registerMessageHandler(caller.toWeakRef());
        sender->callProcedureSlot(name, params);
        return caller;
    }
    return nullptr;
}

bool DIALOGProcess::sendCommand(const QString &name, const QByteArray &message)
{
    if (server->waitForConnectionToControlServer()) {
        QByteArray* commandMessage = new QByteArray();
        commandMessage->append(message);
        sender->sendCommandSlot(name, commandMessage);
    }
    return true;
}

bool DIALOGProcess::sendDirectCommand(const QString &name,
                                      const QByteArray &message,
                                      const QString &processName)
{
    if (server && server->waitForConnectionToControlServer()) {
        QByteArray* commandMessage = new QByteArray();
        commandMessage->append(message);
        sender->sendDirectCommandSlot(name, commandMessage, processName);
    }
    return true;
}

bool DIALOGProcess::sendDirectCommand(const QString &name, const QByteArray &message, const QString &address, quint16 port)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(message);
    sender->sendDirectCommandUrlSlot(name, commandMessage, address, port);
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

bool DIALOGProcess::SenderThread::registerMessageSender(QWeakPointer<DIALOGCommandHandler> sender)
{
    return registerMessageSenderImpl<DIALOGCommandHandler>(
                                                         sender,
                                                         commandsToRegister,
                                                         DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::SenderThread::registerMessageSender(QWeakPointer<DIALOGServiceSubscriber> sender)
{
    return registerMessageSenderImpl<DIALOGServiceSubscriber>(
                                                       sender,
                                                       servicesToRequest,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

bool DIALOGProcess::SenderThread::registerMessageSender(QWeakPointer<DIALOGServicePublisher> sender)
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

bool DIALOGProcess::SenderThread::registerMessageSender(QWeakPointer<DIALOGProcedureCaller> sender)
{
    // Sender thread does not work with procedure callers yet.
    return true;
}

bool DIALOGProcess::SenderThread::registerMessageSender(QWeakPointer<DIALOGProcedurePublisher> sender)
{
    if (registerMessageSenderImpl<DIALOGProcedurePublisher>(
                                                     sender,
                                                     proceduresToRegister,
                                                     DIALOGMessageHandlerType::ProcedurePublisher))
    {
        connect(sender.toStrongRef().data(), &DIALOGProcedurePublisher::callFinishedSignal,
                this, &SenderThread::sendProcedureReturnSlot);
        return true;
    }
    return false;
}

void DIALOGProcess::SenderThread::sendCommandSlot(QString name, QByteArray *message)
{
    emit sendCommandMessageSignal(name, message);
}

void DIALOGProcess::SenderThread::sendDirectCommandSlot(QString name,
                                                        QByteArray *message,
                                                        QString processName)
{
    emit sendDirectCommandMessageSignal(name, message, processName);
}

void DIALOGProcess::SenderThread::sendDirectCommandUrlSlot(QString name,
                                                           QByteArray *message,
                                                           QString address,
                                                           quint16 port)
{
    emit sendDirectCommandUrlMessageSignal(name, message, address, port);
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

void DIALOGProcess::SenderThread::sendProcedureReturnSlot(QString name,
                                                          QByteArray data,
                                                          QString address,
                                                          int port)
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

bool DIALOGProcess::ReceiverThread::registerMessageHandler(
                                                         QWeakPointer<DIALOGCommandHandler> handler)
{
    return registerMessageHandlerImpl<DIALOGCommandHandler>(handler,
                                                            commands,
                                                          DIALOGMessageHandlerType::CommandHandler);
}

bool DIALOGProcess::ReceiverThread::registerMessageHandler(
                                                      QWeakPointer<DIALOGServiceSubscriber> handler)
{
    return registerMessageHandlerImpl<DIALOGServiceSubscriber>(handler,
                                                               serviceSubscribers,
                                                       DIALOGMessageHandlerType::ServiceSubscriber);
}

bool DIALOGProcess::ReceiverThread::registerMessageHandler(
                                                       QWeakPointer<DIALOGServicePublisher> handler)
{
    // Receiver does not work with service publishers yet.
    return true;
}

bool DIALOGProcess::ReceiverThread::registerMessageHandler(
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

bool DIALOGProcess::ReceiverThread::registerMessageHandler(QWeakPointer<DIALOGProcedurePublisher> handler)
{
    return registerMessageHandlerImpl<DIALOGProcedurePublisher>(handler,
                                                               procedurePublishers,
                                                       DIALOGMessageHandlerType::ProcedurePublisher);
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
            if (procedurePublishers.contains(procedure))
            {
                QMetaObject::invokeMethod(procedurePublishers[procedure].toStrongRef().data(),
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

void DIALOGCommandHandler::commandReceivedSlot(QByteArray message)
{
    emit commandReceivedSignal(message);
}

void DIALOGServicePublisher::updateDataSlot(QByteArray dataInit)
{
    data = dataInit;
    emit dataUpdatedSignal(name, data);
}

void DIALOGServiceSubscriber::dataUpdatedSlot(QByteArray dataInit)
{
    data = dataInit;
    emit dataUpdatedSignal(data);
}

QByteArray DIALOGProcedureCaller::waitForData(bool &ok, int timeout)
{
    if (dataSet) {
        return tryGetData();
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &DIALOGProcedureCaller::dataSetSignal, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    timer.start(timeout * 1000);
    loop.exec();

    ok = dataSet;
    return data;
}

QByteArray DIALOGProcedureCaller::tryGetData()
{
    QByteArray value;
    mutex.lock();
    value = data;
    mutex.unlock();
    return value;
}

QString DIALOGProcedureCaller::getName() const
{
    return name;
}

void DIALOGProcedureCaller::setDataSlot(QByteArray dataInit)
{
    mutex.lock();
    data = dataInit;
    dataSet = true;
    mutex.unlock();
    emit dataSetSignal();
}

QString DIALOGProcedurePublisher::getName() const
{
    return name;
}

void DIALOGProcedurePublisher::callRequestedSlot(QByteArray params, QString urlInit, int portInit)
{
    url = urlInit;
    port = portInit;
    emit callRequestedSignal(params);
}

void DIALOGProcedurePublisher::callFinishedSlot(QByteArray dataInit)
{
    data = dataInit;
    emit callFinishedSignal(name, dataInit, url, port);
}

template<typename T>
bool DIALOGProcess::handleMessageTypeImpl(QWeakPointer<T> handler, DIALOGMessageHandlerType type)
{
    auto strongHandler = handler.toStrongRef();
    if (strongHandler)
    {
//        QString name = strongCommand->getName();
//        if (server)
//        {
//            QMetaObject::invokeMethod(server, "registerCommandSlot", Q_ARG(QString, name));
//        }

        if (sender->registerMessageSender(handler))
        {
            return receiver->registerMessageHandler(handler);
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
QSharedPointer<T> DIALOGProcess::handleMessageTypeImpl(const QString &name,
                                                       DIALOGMessageHandlerType type)
{
    QSharedPointer<T> handler = QSharedPointer<T>(new T(name), deleteLater);
    if (handleMessageTypeImpl<T>(handler, type))
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
