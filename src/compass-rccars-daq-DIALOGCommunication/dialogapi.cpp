#include "dialogapi.h"

DIALOGProcess::DIALOGProcess(QString nameInit, QObject *parent)
: QObject(parent)
{
    server = nullptr;

    receiver = new ReceiverThread();
    sender = new SenderThread();

    controlServerUrl = getenv("DIALOG_CONTROL_SERVER_ADDRESS");
    controlServerPort = QString(getenv("DIALOG_CONTROL_SERVER_PORT")).toInt();
    name = nameInit;
}

DIALOGProcess::~DIALOGProcess()
{
    delete server;
    //mozna receiver a sender...
}

void DIALOGProcess::start(QThread::Priority priority)
{
    server = new Server(name, Custom, controlServerUrl, controlServerPort, sender, receiver);
    QObject::connect(server, &Server::destroyed, this, &DIALOGProcess::serverDestroyed);
    server->start(priority);
}

void DIALOGProcess::setControlServerAdress(QString url, int port)
{
    controlServerUrl = url;
    controlServerPort = port;
}

QString DIALOGProcess::getName() const
{
    return name;
}

void DIALOGProcess::registerCommand(DIALOGCommand *command)
{
    receiver->registerCommand(command);
    sender->addCommandToRegister(command->getName());
}

DIALOGCommand *DIALOGProcess::registerCommand(QString name)
{
    DIALOGCommand* command = new DIALOGCommand(name);
    registerCommand(command);
    return command;
}

void DIALOGProcess::registerProcedure(DIALOGProcedureHandler *procedure)
{
    receiver->registerProcedureHandler(procedure);
    sender->addProcedureToRegister(procedure);
}

DIALOGProcedureHandler *DIALOGProcess::registerProcedure(QString name)
{
    DIALOGProcedureHandler* handler = new DIALOGProcedureHandler(name);
    registerProcedure(handler);
    return handler;
}

void DIALOGProcess::registerService(DIALOGServicePublisher *publisher)
{
    //receiver->registerService(publisher);
    sender->addServiceToRegister(publisher);
    sender->addServiceToRequest(publisher->getName());
}

DIALOGServicePublisher *DIALOGProcess::registerService(QString name)
{
    DIALOGServicePublisher* publisher = new DIALOGServicePublisher(name);
    sender->addServiceToRegister(publisher);
    return publisher;
}

void DIALOGProcess::requestService(DIALOGServiceSubscriber *subscriber)
{
    receiver->registerService(subscriber);
    sender->addServiceToRequest(subscriber->getName());
}

DIALOGServiceSubscriber *DIALOGProcess::requestService(QString name)
{
    DIALOGServiceSubscriber* subscriber = new DIALOGServiceSubscriber(name);
    receiver->registerService(subscriber);
    sender->addServiceToRequest(name);
    return subscriber;
}

DIALOGProcedureCaller *DIALOGProcess::callProcedure(QString name, QByteArray message)
{
    DIALOGProcedureCaller* caller = new DIALOGProcedureCaller(name);
    receiver->registerProcedureCaller(caller);
    sender->callProcedureSlot(name, message);
    return caller;
}

void DIALOGProcess::sendCommandSlot(QString name, QByteArray message)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(message);
    sender->sendCommandSlot(name, commandMessage);
}

void DIALOGProcess::sendDirectCommandSlot(QString name, QByteArray message, QString processName)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(message);

    sender->sendDirectCommandSlot(name, commandMessage, processName);
}

void DIALOGProcess::sendDirectCommandSlot(QString name, QByteArray message, QString url, int port)
{
    QByteArray* commandMessage = new QByteArray();
    commandMessage->append(message);
    sender->sendDirectCommandUrlSlot(name, commandMessage, url, port);
}

void DIALOGProcess::stopSlot()
{
    server->stopSlot();
}

DIALOGProcess::SenderThread::~SenderThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Sender destructor";
}

void DIALOGProcess::SenderThread::run()
{
    server->connectToControlServerSlot();
//QThread::sleep(1);
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

    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Sender EventLoop";
}

void DIALOGProcess::SenderThread::addServiceToRequest(QString name)
{
    servicesToRequest.append(name);
}

void DIALOGProcess::SenderThread::addServiceToRegister(DIALOGServicePublisher *publisher)
{
    connect(publisher, &DIALOGServicePublisher::dataUpdatedSignal, this, &SenderThread::sendServiceSlot);
    servicesToRegister.append(publisher->getName());
}

void DIALOGProcess::SenderThread::addCommandToRegister(QString name)
{
    commandsToRegister.append(name);
}

void DIALOGProcess::SenderThread::addProcedureToRegister(DIALOGProcedureHandler *handler)
{
    proceduresToRegister.append(handler->getName());
    connect(handler, &DIALOGProcedureHandler::callFinishedSignal, this, &SenderThread::sendProcedureReturnSlot);

}

void DIALOGProcess::SenderThread::sendCommandSlot(QString name, QByteArray *message)
{
    emit sendCommandMessageSignal(name, message);
}

void DIALOGProcess::SenderThread::sendDirectCommandSlot(QString name, QByteArray *message, QString processName)
{
    emit sendDirectCommandMessageSignal(name, message, processName);
}

void DIALOGProcess::SenderThread::sendDirectCommandUrlSlot(QString name, QByteArray *message, QString url, int port)
{
    emit sendDirectCommandUrlMessageSignal(name, message, url, port);
}

void DIALOGProcess::SenderThread::sendServiceSlot(QString name, QByteArray message)
{
    QByteArray* serviceMessage = new QByteArray();
    serviceMessage->append(message);
    emit sendServiceMessageSignal(name, serviceMessage);
}

void DIALOGProcess::SenderThread::callProcedureSlot(QString name, QByteArray message)
{
    QByteArray* serviceMessage = new QByteArray();
    serviceMessage->append(message);
    emit sendProcedureCallMessageSignal(name, serviceMessage);
}

void DIALOGProcess::SenderThread::sendProcedureReturnSlot(QString name, QByteArray data, QString url, int port)
{
    QByteArray* serviceMessage = new QByteArray();
    serviceMessage->append(data);
    emit sendProcedureReturnMessageSignal(name,serviceMessage,url,port);
}

DIALOGProcess::ReceiverThread::~ReceiverThread()
{
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Receiver destructor";
}

void DIALOGProcess::ReceiverThread::run()
{
    virtualThreadEventLoop = new QEventLoop();
    virtualThreadEventLoop->exec();

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Receiver EventLoop";
}

void DIALOGProcess::ReceiverThread::registerCommand(DIALOGCommand *command)
{
    if (!commands.contains(command->getName())){
        commands[command->getName()] = command;
    }
}

void DIALOGProcess::ReceiverThread::registerService(DIALOGServiceSubscriber *subscriber)
{
    if (!subscribers.contains(subscriber->getName())){
        subscribers[subscriber->getName()] = subscriber;
    }
}

void DIALOGProcess::ReceiverThread::registerProcedureCaller(DIALOGProcedureCaller *caller)
{
    procedureCalls[caller->getName()] = caller;
}

void DIALOGProcess::ReceiverThread::registerProcedureHandler(DIALOGProcedureHandler *handler)
{
    if (!procedureHandlers.contains(handler->getName())) {
        procedureHandlers[handler->getName()] = handler;
    }
}

void DIALOGProcess::ReceiverThread::messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray *header, QByteArray *message)
{
    if (header == NULL || header->data() == NULL || header->size() == 0)
    {
        std::cerr << "Corrupted message received." << std::endl;
        return;
    }

    QList<QByteArray> headerList = header->split(SEPARATOR);
    if(headerList.size() == 0)
    {
        std::cerr << "The separator is not found in message." << std::endl;
        return;
    }

    if(headerList[0] == COMMAND_MESSAGE)
    {
        QString command = headerList[1];
        if (commands.contains(command)) {

            QMetaObject::invokeMethod(commands[command], "commandReceivedSlot",
                                      Q_ARG( QByteArray, *message ) );
 //           commands[command]->commandReceivedSlot(*message);
        }
        else {
            std::cerr << "Unknown command arrived." << std::endl;
        }
    }
    else if (headerList[0] == SERVICE_MESSAGE)
    {
        QString service = headerList[1];
        if(subscribers.contains(service)){
            //subscribers[service]->dataUpdatedSlot(*message);

            QMetaObject::invokeMethod(subscribers[service], "dataUpdatedSlot",
                                      Q_ARG( QByteArray, *message ) );
        }
        else {
            std::cerr << "Unknown service arrived." << std::endl;
        }
    }
    else if (headerList[0] == PROCEDURE_MESSAGE)
    {
        if (headerList[1] == PROCEDURE_CALL){
            QString procedure = headerList[2];
            if(procedureHandlers.contains(procedure)){
                //procedureHandlers[procedure]->callRequestedSlot(*message, headerList[3], headerList[4].toInt());
                QMetaObject::invokeMethod(procedureHandlers[procedure], "callRequestedSlot",
                                          Q_ARG( QByteArray, *message ), Q_ARG( QString, headerList[3] ), Q_ARG( int, headerList[4].toInt() ) );
            }
            else {
                std::cerr << "Unknown procedure call arrived." << std::endl;
            }
        }
        else if (headerList[1] == PROCEDURE_DATA) {
            QString procedure = headerList[2];

            if(procedureCalls.contains(procedure)){
                //procedureCalls[procedure]->setDataSlot(*message);//bude cekat??
                QMetaObject::invokeMethod(procedureCalls[procedure], "setDataSlot",
                                          Q_ARG( QByteArray, *message ) );
            }
            else {
                std::cerr << "Unknown procedure data arrived." << std::endl;
            }
        }
        else {
            std::cerr << "Unknown procedure message arrived." << std::endl;
        }
    }
    delete header;
    delete message;
}

void DIALOGCommand::commandReceivedSlot(QByteArray message)
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
    qDebug() << QString(dataInit);
    emit dataUpdatedSignal(dataInit);
}

QByteArray DIALOGProcedureCaller::waitForData(bool &ok, int timeout)
{
    if(dataSet) {
        return tryGetData();
    }

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &DIALOGProcedureCaller::dataSetSignal, &loop, &QEventLoop::quit );
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

QString DIALOGProcedureHandler::getName() const
{
    return name;
}

void DIALOGProcedureHandler::callRequestedSlot(QByteArray params, QString urlInit, int portInit)
{
    url = urlInit;
    port = portInit;
    emit callRequestedSignal(params);
}

void DIALOGProcedureHandler::callFinishedSlot(QByteArray dataInit)
{
    data = dataInit;
    emit callFinishedSignal(name, dataInit, url, port);
}
