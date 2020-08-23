#include "server.h"
#include <QRegularExpression>
#include <unistd.h>

void Server::infoService(const QStringList& messageList)
{

    if(services.contains(messageList[0]))
    {
        if(services[messageList[0]]->sender == NULL)
        {
            if (messageList[1] != "unknown")
            {
                Service* service = services[messageList[0]];
                QString senderProcessKey = messageList[1] + SEPARATOR + messageList[2];
                if(isProcessKnown(senderProcessKey))
                {
                    service->addSender(getProcess(senderProcessKey));
                    getProcess(senderProcessKey)->addServiceAsSender(service);
                }
                else
                {
                    Process* senderProcess = new Process(messageList[1],
                                                         messageList[2].toUShort(),
                                                         Custom);

                    addProcess(senderProcessKey, senderProcess);
                    service->addSender(senderProcess);
                    senderProcess->addServiceAsSender(service);
                }

                subscribeServiceSlot(messageList[0]);
            }
            else
            {
                DIALOGCommon::logMessage(QString("CommunicationControlServer does not know "
                                                 "the sender of service %1.")
                                         .arg(QString(messageList[0])));
            }
        }
        else
            DIALOGCommon::logMessage(QString("Service is already known.")
                                     .arg(QString(messageList[0])));
    }
    else
        DIALOGCommon::logMessage(QString("Service is not known on the server.").arg(QString(messageList[0])));
}

void Server::subscribeService(const QString& senderKey, const QStringList &messageList)
{
    if(services.contains(messageList[0]))
    {
        bool alreadySubscribed = false;
        if(isProcessKnown(senderKey))
        {
            alreadySubscribed = !services[messageList[0]]->addReceiver(getProcess(senderKey));
            getProcess(senderKey)->addServiceAsReceiver(services[messageList[0]]);
        }
        else
        {
            Process* receiverProcess = getProcess(senderKey);
            receiverProcess->processType = static_cast<ProcessType>(messageList[1].toInt());
            alreadySubscribed = !services[messageList[0]]->addReceiver(receiverProcess);
            getProcess(senderKey)->addServiceAsReceiver(services[messageList[0]]);
        }
        if(!alreadySubscribed)
            DIALOGCommon::logMessage(QString("Service %1 has been subscribed by (%2)")
                                     .arg(messageList[0]).arg(senderKey));
        else
            DIALOGCommon::logMessage(QString("Service %1 has already been subscribed by (%2)")
                                     .arg(messageList[0]).arg(senderKey));
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 is not provided on the server.")
                                 .arg(messageList[0]));
}

void Server::unsubscribeService(const QStringList &messageList)
{
    QString receiverKey = messageList[1] + SEPARATOR + messageList[2];
    if(services.contains(messageList[0]))
    {
        Service* service = services[messageList[0]];
        if(isProcessKnown(receiverKey))
        {
            Process* receiverProcess = getProcess(receiverKey);
            if(service->sender == serverProcess)
            {
                receiverProcess->removeServiceAsReceiver(service);
                service->removeReceiver(receiverProcess);
                if(!hasProcessDependencies(receiverProcess))
                {
                    removeProcess(receiverProcess->processKey);
                }
                DIALOGCommon::logMessage(QString("Service %1 has been unsubscribed by (%2,%3).")
                                         .arg(messageList[0])
                                         .arg(messageList[1])
                                         .arg(messageList[2]));
            }
            else
            {
                DIALOGCommon::logMessage(QString("This process (%1,%2) is not the "
                                                 "sender for Service %3.")
                                         .arg(serverProcess->processAddress)
                                         .arg(serverProcess->processPort)
                                         .arg(messageList[0]));
            }
        }
        else
        {
            DIALOGCommon::logMessage(QString("Process (%1) is not known on the server.")
                                     .arg(receiverKey));
        }
    }
    else
        DIALOGCommon::logMessage(QString("Service (%1) is not provided on the server.")
                                 .arg(receiverKey));
}

void Server::lostSender(const QStringList &messageList)
{
    if(services.contains(messageList[0]))
    {
        Service* service = services[messageList[0]];
        Process* serviceSender = service->sender;
        if(serviceSender != NULL)
        {
            service->removeSender();
            serviceSender->removeServiceAsSender(service);
            if(!hasProcessDependencies(serviceSender))
            {
                removeProcess(serviceSender->processKey);
            }
            DIALOGCommon::logMessage(QString("Lost sender with service %1.").arg(messageList[0]));
        }
        else
            DIALOGCommon::logMessage(QString("The sender with service %1 has not been already "
                                             "known. It could not be deleted.")
                                     .arg(messageList[0]));
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 is not known on the server.")
                                 .arg(messageList[0]));
}

void Server::lostReceiver(const QStringList &messageList)
{
    foreach (Service* service, services)
    {
        foreach (Process* receiverProcess, service->receivers)
        {
            if (receiverProcess->processAddress == messageList[0] &&
                receiverProcess->processPort == messageList[1].toUShort())
            {
                DIALOGCommon::logMessage(QString("Lost Receiver (%1, %2).")
                                        .arg(messageList[0])
                                        .arg(messageList[1]));

                service->removeReceiver(receiverProcess);
                receiverProcess->removeServiceAsReceiver(service);
                if(!hasProcessDependencies(receiverProcess))
                {
                    removeProcess(receiverProcess->processKey);
                }
                break;
            }
        }
    }
}

void Server::connectRequest(const QString &senderAddress,
                            quint16 senderPort,
                            const QString &senderKey,
                            const QStringList &messageList)
{
    if(!isProcessConnectedToControlServer(senderKey))
    {
        Process* process = getProcess(senderKey);
        process->processType = static_cast<ProcessType>(messageList[0].toInt());
        process->processName = messageList[1];
        process->processPID = messageList[2].toULongLong();
        process->connectedToControlServer = true;
        Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(SUCCESSFULY_CONNECTED));

        if(process->processType == Custom)
            DIALOGCommon::logMessage(QString("The process %1 (%2, %3) is successfully connected "
                                             "to CommunicationControlServer.")
                                     .arg(QString(messageList[1]))
                                     .arg(senderAddress)
                                     .arg(senderPort));

        else
            DIALOGCommon::logMessage(QString("A new Monitoring (%1, %2) is successfully connected "
                                             "to CommunicationControlServer.")
                                    .arg(senderAddress)
                                    .arg(senderPort));

        infoMonitoringSlot();
    }
    else
    {
        DIALOGCommon::logMessage(QString("The process %1 (%2, %3) has been already connected to"
                                         " CommunicationControlServer.")
                                 .arg(QString(messageList[1]))
                                 .arg(senderAddress)
                                 .arg(senderPort));
    }
    processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();
}

void Server::registerService(const QString &senderAddress,
                             quint16 senderPort,
                             Process* sender,
                             const QStringList &messageList)
{
    if(!services.contains(messageList[0]))
    {
        Service* service = new Service(messageList[0]);
        service->addSender(sender);
        sender->addServiceAsSender(service);
        services[messageList[0]] = service;
        DIALOGCommon::logMessage(QString("New Service %1 has been registered on "
                                         "CommunicationControlServer.")
                                 .arg(messageList[0]));
        infoMonitoringSlot();
    }
    else
    {
        Service* service = services[messageList[0]];
        if(service->sender == NULL)
        {
            service->addSender(sender);
            sender->addServiceAsSender(service);

            foreach(Process* receiver, service->receivers)
            {
                if(receiver->processType == Custom)
                {
                    QByteArray* infoService = new QByteArray();
                    infoService->append(service->serviceName);
                    infoService->append(SEPARATOR);
                    infoService->append(service->sender->processAddress);
                    infoService->append(SEPARATOR);
                    infoService->append(QString::number(service->sender->processPort));
                    Q_EMIT sendMessageSignal(receiver->processAddress,
                                             receiver->processPort,
                                             messageHeader(INFO_SERVICE),
                                             infoService);
                }
            }
            DIALOGCommon::logMessage(QString("New sender of service %1 has been registered on "
                                             "CommunicationControlServer. Info about service %1 "
                                             "has been sent.")
                                     .arg(messageList[0]));
            infoMonitoringSlot();
        }
        else
        {
            QByteArray* dialogError = new QByteArray();
            dialogError->append(service->serviceName);

            Q_EMIT sendMessageSignal(senderAddress,
                                     senderPort,
                                     messageHeader(QString(DIALOG_ERROR)
                                                   .append(SEPARATOR)
                                                   .append(SERVICE_DUPLICATE)),
                                     dialogError);

            DIALOGCommon::logMessage(QString("Service %1 is already registered on ControlSever.")
                                    .arg(messageList[0]));
        }
    }

}

void Server::requestService(const QString &senderAddress,
                            quint16 senderPort,
                            Process *sender,
                            const QStringList &messageList)
{
    QByteArray* infoService = new QByteArray();
    infoService->append(messageList[0]);
    infoService->append(SEPARATOR);
    if(services.contains(messageList[0]))
    {
        services[messageList[0]]->addReceiver(sender);
        sender->addServiceAsReceiver(services[messageList[0]]);
        Process* senderProcess = services[messageList[0]]->sender;
        if(senderProcess != NULL)
        {
            infoService->append(senderProcess->processAddress);
            infoService->append(SEPARATOR);
            infoService->append(QString::number(senderProcess->processPort));
            DIALOGCommon::logMessage(QString("Info about service %1 has been sent.")
                                     .arg(messageList[0]));
        }
        else
        {
            infoService->append("unknown");
            DIALOGCommon::logMessage(QString("Requested service %1 has no sender on ControlSever.")
                                     .arg(messageList[0]));
        }
    }
    else
    {
        Service* service = new Service(messageList[0]);
        service->addReceiver(sender);
        sender->addServiceAsReceiver(service);
        services[messageList[0]] = service;

        infoService->append("unknown");
        DIALOGCommon::logMessage(QString("Requested service %1 has no sender on"
                                         " CommunicationControlSever.")
                                 .arg(messageList[0]));
    }

    Q_EMIT sendMessageSignal(senderAddress,
                             senderPort,
                             messageHeader(INFO_SERVICE),
                             infoService);

    if(sender->processType != Monitoring)
        infoMonitoringSlot();
}

void Server::unsubscribeService(const QString& senderAddress,
                                quint16 senderPort,
                                Process *sender,
                                const QStringList &messageList)
{
    if(services.contains(messageList[0]))
    {
        Service* service = services[messageList[0]];
        if(sender->servicesAsReceiver.contains(service))
        {
            sender->removeServiceAsReceiver(service);
            service->removeReceiver(sender);

            if(service->sender != NULL)
            {
                QByteArray* unsubscribeService = new QByteArray();
                unsubscribeService->append(messageList[0]);
                unsubscribeService->append(SEPARATOR);
                unsubscribeService->append(sender->processAddress);
                unsubscribeService->append(SEPARATOR);
                unsubscribeService->append(QString::number(sender->processPort));

                Q_EMIT sendMessageSignal(service->sender->processAddress,
                                         service->sender->processPort,
                                         messageHeader(UNSUBSCRIBE_SERVICE),
                                         unsubscribeService);
            }

            DIALOGCommon::logMessage(QString("Service %1 has been unsubscribed by (%2, %3).")
                                     .arg(messageList[0])
                                     .arg(senderAddress)
                                     .arg(senderPort));

            if(sender->processType != Monitoring)
                infoMonitoringSlot();
        }
        else
        {
            DIALOGCommon::logMessage(QString("Service %1 is not subscribed by (%2, %3).")
                                     .arg(messageList[0])
                                     .arg(senderAddress)
                                     .arg(senderPort));
        }
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 s not provided on the "
                                         "CommunicationControlServer.")
                                 .arg(messageList[0]));
}

void Server::listOfServices(const QString &senderAddress,
                            quint16 senderPort,
                            const QStringList headerList)
{
    quint16 numberOfAvailableServices = 0;
    QByteArray* listOfAvailableServices = new QByteArray();
    bool first = true;
    foreach(QString key, services.keys())
    {
        if (services[key]->sender != NULL)
        {
            if(key.contains(QRegularExpression(headerList[1])))
            {
                numberOfAvailableServices++;
                if(!first)
                    listOfAvailableServices->append(SEPARATOR);
                else
                    first = false;
                listOfAvailableServices->append(key);
            }
        }
    }
    Q_EMIT sendMessageSignal(senderAddress,
                             senderPort,
                             messageHeader(headerList.join(SEPARATOR)),
                             listOfAvailableServices);
}

void Server::registerCommand(const QString &senderAddress,
                             quint16 senderPort,
                             Process *sender,
                             const QStringList &messageList)
{
    if(!commands.contains(messageList[0]))
    {
        Command* command = new Command(messageList[0]);
        command->addReceiver(sender);
        sender->addCommand(command);
        commands[messageList[0]] = command;
        DIALOGCommon::logMessage(QString("New command %1 has been registered on "
                                         "CommunicationControlServer.")
                                .arg(messageList[0]));
        infoMonitoringSlot();
    }
    else
    {
        Command* command = commands[messageList[0]];
        if(command->addReceiver(sender))
        {
            sender->addCommand(command);
            DIALOGCommon::logMessage(QString("New receiver of command %1 has been registered"
                                             "on CommunicationControlServer.")
                                    .arg(messageList[0]));
            if(sender->processType != Monitoring)
                infoMonitoringSlot();
        }
        else
            DIALOGCommon::logMessage(QString("Command %1 is already registered by (%2, %3) "
                                             "on CommunicationControlServer.")
                                    .arg(messageList[0])
                                    .arg(senderAddress)
                                    .arg(senderPort));
    }
}

void Server::unregisterCommand(const QString &senderAddress,
                               quint16 senderPort,
                               Process *sender,
                               const QStringList &messageList)
{
    if(commands.contains(messageList[0]))
    {
        Command* command = commands[messageList[0]];
        if(sender->commands.contains(command))
        {
            sender->removeCommand(command);
            command->removeReceiver(sender);
            DIALOGCommon::logMessage(QString("Command %1 has been unregistered by (%2, %3) ")
                                    .arg(messageList[0])
                                    .arg(senderAddress)
                                    .arg(senderPort));
            if(sender->processType != Monitoring)
                infoMonitoringSlot();
        }
        else
        {
            DIALOGCommon::logMessage(QString("Command %1 is not registered by (%2, %3) ")
                                    .arg(messageList[0])
                                    .arg(senderAddress)
                                    .arg(senderPort));
        }
    }
    else
        DIALOGCommon::logMessage(QString("Command %1 is not provided on the"
                                         " CommunicationControlServer.")
                                 .arg(messageList[0]));
}

void Server::commandMessage(const QStringList &headerList, QByteArray *message)
{
    if(headerList[1] != DIRECT) {
        if(commands.contains(headerList[1]))
        {
            Command* command = commands[headerList[1]];
            foreach(Process* receiver, command->receivers)
            {
                QByteArray* newMessage = new QByteArray();
                newMessage->append(*message);
                Q_EMIT sendMessageSignal(receiver->processAddress,
                                         receiver->processPort,
                                         messageHeader(headerList.join(SEPARATOR)),
                                         newMessage);
                DIALOGCommon::logMessage(QString("Command %1 has been received on "
                                                 "CommunicationControlServer and sent to (%2, %3).")
                        .arg(headerList[1])
                        .arg(receiver->processAddress)
                        .arg(receiver->processPort));
            }
        }
        else
        {
            DIALOGCommon::logMessage(QString("Unknown command . The command is not registered "
                                             "on CommunicationControlSever.")
                                     .arg(headerList[1]));
        }
    }
    else {
        if(commands.contains(headerList[3]))
        {
            Command* command = commands[headerList[3]];
            bool send = false;
            foreach(Process* receiver, command->receivers)
            {
                if (receiver->processName == headerList[2]){
                    QByteArray* newMessage = new QByteArray();
                    newMessage->append(*message);

                    QString newHeader(headerList[0]);
                    newHeader.append(SEPARATOR);
                    newHeader.append(headerList[3]);

                    Q_EMIT sendMessageSignal(receiver->processAddress,
                                             receiver->processPort,
                                             messageHeader(newHeader),
                                             newMessage);

                    DIALOGCommon::logMessage(QString("Command %1 has been received on "
                                                     "CommunicationControlServer "
                                                     "and sent to (%2, %3).")
                            .arg(headerList[1])
                            .arg(receiver->processAddress)
                            .arg(receiver->processPort));
                    send = true;
                }
            }
            if (!send){
                DIALOGCommon::logMessage(QString("Process does not register command %1.")
                                         .arg(headerList[3]));
            }
        }
        else
        {
            DIALOGCommon::logMessage(QString("Unknown command %1. The command is not "
                                             "registered on CommunicationControlSever.")
                                     .arg(headerList[3]));
        }
    }
}

void Server::registerProcesure(const QString &senderAddress,
                               quint16 senderPort,
                               Process *sender,
                               const QStringList &messageList)
{
    if(!procedures.contains(messageList[0]))
    {
        Procedure* procedure = new Procedure(messageList[0]);
        procedure->addSender(sender);
        sender->addProcedure(procedure);
        procedures[messageList[0]] = procedure;
        DIALOGCommon::logMessage(QString("New procedure %1 has been registered "
                                         "on CommunicationControlServer.")
                                 .arg(messageList[0]));
        infoMonitoringSlot();
    }
    else
    {
        Procedure* procedure = procedures[messageList[0]];
        if(procedure->addSender(sender))
        {
            sender->addProcedure(procedure);
            DIALOGCommon::logMessage(QString("New sender of Procedure  has been "
                                             "registered on CommunicationControlServer.")
                    .                arg(messageList[0]));;

            if(sender->processType != Monitoring)
                infoMonitoringSlot();
        }
        else
            DIALOGCommon::logMessage(QString("Procedure %1 is already registered by (%2, %3).")
                                    .arg(messageList[0])
                                    .arg(senderAddress)
                                    .arg(senderPort));
    }
}

void Server::unregisterProcedure(const QString &senderAddress,
                                 quint16 senderPort,
                                 Process *sender,
                                 const QStringList &messageList)
{
    if(procedures.contains(messageList[0]))
    {
        Procedure* procedure = procedures[messageList[0]];
        if(sender->procedures.contains(procedure))
        {
            sender->removeProcedure(procedure);
            DIALOGCommon::logMessage(QString("Procedure %1 has been unregistered by (%2, %3)"
                                             "on CommunicationControlServer.")
                                     .arg(messageList[0])
                                     .arg(senderAddress)
                                     .arg(senderPort));
            if(sender->processType != Monitoring)
                infoMonitoringSlot();
        }
        else
        {
            DIALOGCommon::logMessage(QString("Procedure %1 is not registered by (%2, %3)")
                                     .arg(messageList[0])
                                     .arg(senderAddress)
                                     .arg(senderPort));
        }
    }
    else
        DIALOGCommon::logMessage(QString("Procedure %1 is not provided "
                                         "on CommunicationControlServer.")
                                 .arg(messageList[0]));
}

void Server::procedureMessage(const QString& senderAddress,
                              quint16 senderPort,
                              const QStringList &headerList,
                              QByteArray *message)
{
    if(headerList[1] == PROCEDURE_CALL) {
        if(procedures.contains(headerList[2]))
        {
            Procedure* procedure = procedures[headerList[2]];
            //chyby distribuce...
            Process* receiver = procedure->senders[0];

            QByteArray* newMessage = new QByteArray();
            newMessage->append(*message);

            QString newHeader(headerList[0]);
            newHeader.append(SEPARATOR);
            newHeader.append(headerList[1]);
            newHeader.append(SEPARATOR);
            newHeader.append(headerList[2]);
            newHeader.append(SEPARATOR);
            newHeader.append(senderAddress);
            newHeader.append(SEPARATOR);
            newHeader.append(QString::number(senderPort));

            Q_EMIT sendMessageSignal(receiver->processAddress,
                                     receiver->processPort,
                                     messageHeader(newHeader),
                                     newMessage);

            DIALOGCommon::logMessage(QString("Procedure %1 as been received on "
                                             "CommunicationControlServer and sent to (%2, %3)")
                                     .arg(headerList[2])
                                     .arg(senderAddress)
                                     .arg(senderPort));
        }
        else
        {
            DIALOGCommon::logMessage(QString("Unknown Procedure %1. The procedure is not "
                                             "registered on CommunicationControlSever.")
                                     .arg(headerList[2]));
        }
    }
}

void Server::unknownMessage(const QString &senderAddress,
                            quint16 senderPort,
                            QByteArray *header,
                            QByteArray *message)
{
    QString unknownMessage = QString("Unknown message from (%1, %2): %3")
                                     .arg(senderAddress)
                                     .arg(senderPort)
                                     .arg(QString(*header));
    if(message) {
        unknownMessage.append(*message);
    }
    DIALOGCommon::logMessage(unknownMessage);
}

Server::Server(QString serverNameInit,
               ProcessType processTypeInit,
               QString controlServerAddressInit,
               quint16 controlServerPortInit,
               VirtualThread* senderThreadInit,
               VirtualThread* receiverThreadInit)
{
    processType = processTypeInit;
    serverName = serverNameInit;

    controlServerAddress = controlServerAddressInit;
    controlServerPort = controlServerPortInit;
    connectedToControlServer = false;

    senderThread = senderThreadInit;
    receiverThread = receiverThreadInit;

    if(processType != ControlServer)
    {
        QHostInfo info = QHostInfo::fromName(getenv("HOSTNAME"));
        if (!info.addresses().isEmpty()){
            QHostAddress address = info.addresses().first();
            serverAddress = address.toString();
        }
        else {
            serverAddress = QString(getenv("HOSTNAME"));
        }
        serverPort = 0;

        if(serverAddress.length() == 0)
        {
            char hostname[1024];
            gethostname(hostname, 1024);
            serverAddress = QString(hostname);
            if(serverAddress.length() == 0)
            {
                DIALOGCommon::logMessage("ServerAddress could not be detected.");
                hardStopSlot();
            }
        }
    }
    else
    {
        serverAddress = controlServerAddressInit;
        serverPort = controlServerPortInit;
    }

    mainThread = currentThread();
    moveToThread(this);
}

void Server::run()
{
    controlServer = new Process(controlServerAddress, controlServerPort, ControlServer);
    receiver = new Receiver(serverAddress, serverPort, this);
    sender = new Sender(this);

    QObject::connect(this, &Server::sendMessageSignal, sender,
                     &Sender::sendMessageSlot, Qt::DirectConnection);
    QObject::connect(this, &Server::sendHeartBeatSignal,
                     sender, &Sender::sendHeartBeatSlot);
    QObject::connect(sender, &Sender::senderErrorSignal,
                     this, &Server::serverErrorSlot);

    /* TIMERS */
    if(processType != ControlServer)
    {
        connectedToControlServer = false;
        heartBeatTimer = new QTimer();
        heartBeatCheckerTimer = NULL;
        reConnectionTimer = new QTimer();
        QObject::connect(heartBeatTimer, &QTimer::timeout,
                         this, &Server::sendHeartBeatSlot);
        QObject::connect(reConnectionTimer, &QTimer::timeout,
                         this, &Server::reConnectToControlServerSlot);

        QObject::connect(receiverThread, &VirtualThread::started,
                         receiver, &Receiver::startThread, Qt::DirectConnection);
        QObject::connect(receiver, &Receiver::tcpServerStartedSignal,
                         this, &Server::receiverStartSlot);
        QObject::connect(this, &Server::receiverStartedSignal,
                         sender, &Sender::startThread, Qt::DirectConnection);
        QObject::connect(sender, &Sender::senderStartedSignal,
                         senderThread, &VirtualThread::startThread, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendServiceMessageSignal,
                         sender, &Sender::sendServiceMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendCommandMessageSignal,
                         sender, &Sender::sendCommandMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendDirectCommandMessageSignal,
                         sender, &Sender::sendDirectCommandMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendDirectCommandUrlMessageSignal,
                         sender, &Sender::sendDirectCommandUrlMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendProcedureCallMessageSignal,
                         sender, &Sender::callProcedureMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendProcedureReturnMessageSignal,
                         sender, &Sender::sendProcedureReturnMessageSlot, Qt::DirectConnection);

        QObject::connect(this, &Server::serverErrorSignal,
                         receiverThread, &VirtualThread::serverErrorSlot);

        QObject::connect(this, &Server::stopInitiated,
                         senderThread, &VirtualThread::stop);
        QObject::connect(senderThread, &VirtualThread::finished,
                         senderThread, &QObject::deleteLater);
        QObject::connect(senderThread, &VirtualThread::destroyed,
                         sender, &Sender::stop);
        QObject::connect(sender, &Sender::finished,
                         sender, &QObject::deleteLater);
        QObject::connect(sender, &Sender::destroyed,
                         receiver, &Receiver::stop);
        QObject::connect(receiver, &Sender::finished,
                         receiver, &QObject::deleteLater);
        QObject::connect(receiver, &Sender::destroyed,
                         receiverThread, &VirtualThread::stop);
        QObject::connect(receiverThread, &VirtualThread::finished,
                         receiverThread, &QObject::deleteLater);
        QObject::connect(receiverThread, &VirtualThread::destroyed,
                         this, &Server::stopServerSlot);
        QObject::connect(this, &Server::finished,
                         this, &Server::deleteLater);

        senderThread->setServer(this);
        receiverThread->setServer(this);

        receiverThread->start();
    }
    else
    {
        connectedToControlServer = true;
        heartBeatTimer = NULL;
        heartBeatCheckerTimer = new QTimer();
        reConnectionTimer = NULL;
        QObject::connect(heartBeatCheckerTimer, &QTimer::timeout,
                         this, &Server::checkHeartBeatSlot);
        heartBeatCheckerTimer->start(HEARTBEAT_CHECKER_TIMER);

        QObject::connect(receiver, &Receiver::tcpServerStartedSignal,
                         this, &Server::receiverStartSlot);
        QObject::connect(this, &Server::receiverStartedSignal,
                         sender, &Sender::startThread, Qt::DirectConnection);

        QObject::connect(this, &Server::stopInitiated, sender, &Sender::stop);
        QObject::connect(sender, &Sender::finished, sender, &QObject::deleteLater);
        QObject::connect(sender, &Sender::destroyed, receiver, &Receiver::stop);
        QObject::connect(receiver, &Sender::finished, receiver, &QObject::deleteLater);
        QObject::connect(receiver, &Sender::destroyed, this, &Server::stopServerSlot);
        QObject::connect(this, &Server::finished, this, &Server::deleteLater);

        receiver->start();
    }

    serverEventLoop = new QEventLoop();
    serverEventLoop->exec();

    DIALOGCommon::logMessage("End of Server EventLoop");

    if(heartBeatTimer != NULL)
    {
        if(heartBeatTimer->isActive())
            heartBeatTimer->stop();
        delete heartBeatTimer;
    }

    if(heartBeatCheckerTimer != NULL)
    {
        if(heartBeatCheckerTimer->isActive())
            heartBeatCheckerTimer->stop();
        delete heartBeatCheckerTimer;
    }

    if(reConnectionTimer != NULL)
    {
        if(reConnectionTimer->isActive())
            reConnectionTimer->stop();
        delete reConnectionTimer;
    }

    foreach(QByteArray* header, headers)
        delete header;
    foreach(Command* command, commands)
        delete command;
    foreach(Service* service, services)
        delete service;
    foreach(Process* process, processes)
        delete process;

    delete controlServer;
    moveToThread(mainThread);
}

void Server::stop()
{
    QTimer::singleShot(HARD_STOP_TIMER, this, &Server::hardStopSlot);

    Q_EMIT stopInitiated();
}

void Server::stopSlot()
{
    stop();
}

void Server::stopServerSlot()
{
    serverEventLoop->exit();
}

void Server::receiverStartSlot()
{
    serverAddress = receiver->getReceiverAddress();
    serverPort = receiver->getReceiverPort();
    serverProcess = new Process(serverAddress,
                                serverPort,
                                processType,
                                serverName,
                                QCoreApplication::applicationPid());
    addProcess(serverProcess->processKey, serverProcess);

    Q_EMIT receiverStartedSignal();
}

void Server::hardStopSlot()
{
    QString command("kill -9 ");
    command.append(QString::number(QCoreApplication::applicationPid()));

    system(command.toUtf8().data());
}

void Server::messageReceivedSlot(QString senderAddress,
                                 quint16 senderPort,
                                 QByteArray* header,
                                 QByteArray* message)
{
    QString senderKey = senderAddress + SEPARATOR + QString::number(senderPort);

    QStringList headerList = QString(*header).split(SEPARATOR);

    QStringList messageList;
    if (message) {
        messageList = QString(*message).split(SEPARATOR);
    }

    if(serverProcess->processType == ControlServer)
    {
        if(headerList.first() == CONNECT_TO_CONTROL_SERVER)
        {
            connectRequest(senderAddress, senderPort, senderKey, messageList);
        }
        else if(isProcessKnown(senderKey) && isProcessConnectedToControlServer(senderKey))
        {
            Process* sender = getProcess(senderKey);
            if(headerList.first() == REGISTER_SERVICE)
            {
                registerService(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == REQUEST_SERVICE)
            {
                requestService(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == UNSUBSCRIBE_SERVICE)
            {
                unsubscribeService(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == LIST_OF_AVAILABLE_SERVICES)
            {
                listOfServices(senderAddress, senderPort, headerList);
            }
            else if(headerList.first() == REGISTER_COMMAND)
            {
                registerCommand(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == UNREGISTER_COMMAND)
            {
                unregisterCommand(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == COMMAND_MESSAGE)
            {
                commandMessage(headerList, message);
            }
            else if(headerList.first() == REGISTER_PROCEDURE)
            {
                registerProcesure(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == UNREGISTER_PROCEDURE)
            {
                unregisterProcedure(senderAddress, senderPort, sender, messageList);
            }
            else if(headerList.first() == PROCEDURE_MESSAGE)
            {
                procedureMessage(senderAddress, senderPort, headerList, message);
            }
            else if(headerList.first() == HEARTBEAT)
            {
                processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();
            }
            else
            {
                unknownMessage(senderAddress, senderPort, header, message);
            }
        }
        else
        {
            Q_EMIT sendMessageSignal(senderAddress,
                                     senderPort,
                                     messageHeader(CONNECTION_LOST));
            processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();
            DIALOGCommon::logMessage(QString("Unknown sender (%1, %2). The sender is not connected"
                                             " to CommunicationControlServer.")
                                     .arg(senderAddress)
                                     .arg(senderPort));
        }
    }
    else
    {
        if(headerList.first() == SUCCESSFULY_CONNECTED)
        {
            successfullyConnected();
        }
        else if(headerList.first() == CONNECTION_LOST)
        {
            connectionLost();
        }
        else if(headerList.first() == INFO_SERVICE)
        {
            infoService(messageList);
        }
        else if(headerList.first() == SUBSCRIBE_SERVICE)
        {
            subscribeService(senderKey, messageList);
        }
        else if(header->contains(UNSUBSCRIBE_SERVICE))
        {
            unsubscribeService(messageList);
        }
        else if(header->contains(LOST_SENDER))
        {
            lostSender(messageList);
        }
        else if(header->contains(LOST_RECEIVER))
        {
            lostReceiver(messageList);
        }
        else
        {
            unknownMessage(senderAddress, senderPort, header, message);
        }
    }

    delete header;
    if (message) {
        delete message;
    }
}

void Server::connectToControlServerSlot()
{
    if (serverProcess->processType != ControlServer)
    {
        QByteArray* message = new QByteArray();
        message->append(QString::number(serverProcess->processType));
        message->append(SEPARATOR);
        message->append(serverProcess->processName);
        message->append(SEPARATOR);
        message->append(QString::number(serverProcess->processPID));

        Q_EMIT sendMessageSignal(controlServer->processAddress,
                                 controlServer->processPort,
                                 messageHeader(CONNECT_TO_CONTROL_SERVER),
                                 message);

        waitForConnectionToControlServer();
        if (!isConnectedToControlServer()) {
            DIALOGCommon::logMessage("Connection to control server failed.");
        }

    }
    else
    {
        DIALOGCommon::logMessage("ControlServer is trying to connect to itself. "
                                 "It does not make any sense.");
    }
}

void Server::reConnectToControlServerSlot()
{
    connectToControlServerSlot();
}

void Server::registerServiceSlot(QString serviceName)
{
    if(!services.contains(serviceName))
    {
        Service* service = new Service(serviceName);
        services[serviceName] = service;
        service->addSender(serverProcess);
        serverProcess->addServiceAsSender(service);
    }

    if(isConnectedToControlServer())
    {
        QByteArray* message = new QByteArray();
        message->append(serviceName);

        Q_EMIT sendMessageSignal(controlServer->processAddress,
                                 controlServer->processPort,
                                 messageHeader(REGISTER_SERVICE),
                                 message);
    }
}

void Server::requestServiceSlot(QString serviceName)
{
    if(!services.contains(serviceName))
    {
        Service* service = new Service(serviceName);
        services[serviceName] = service;
        service->addReceiver(serverProcess);
        serverProcess->addServiceAsReceiver(service);
    }

    if(isConnectedToControlServer())
    {
        QByteArray* message = new QByteArray();
        message->append(serviceName);

        Q_EMIT sendMessageSignal(controlServer->processAddress,
                                 controlServer->processPort,
                                 messageHeader(REQUEST_SERVICE),
                                 message);
    }
}

void Server::subscribeServiceSlot(QString serviceName)
{
    if(services.contains(serviceName))
    {
        Process* senderProcess = services[serviceName]->sender;

        if (senderProcess != NULL)
        {
            QByteArray* message = new QByteArray();
            message->append(serviceName);
            message->append(SEPARATOR);
            message->append(QString::number(serverProcess->processType));

            Q_EMIT sendMessageSignal(senderProcess->processAddress,
                                     senderProcess->processPort,
                                     messageHeader(SUBSCRIBE_SERVICE),
                                     message);
        }
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 is unknown.").arg(serviceName));
}

void Server::unSubscribeServiceSlot(QString serviceName)
{
    if(services.contains(serviceName))
    {
        Service* service = services[serviceName];
        foreach(Process* receiver, service->receivers)
            receiver->removeServiceAsReceiver(service);
        if(service->sender != NULL)
        {
            service->sender->removeServiceAsSender(service);
            if(!hasProcessDependencies(service->sender))
            {
                removeProcess(service->sender->processKey);
            }
        }

        services.remove(serviceName);
        delete service;

        if(isConnectedToControlServer())
        {
            QByteArray* message = new QByteArray();
            message->append(serviceName);

            Q_EMIT sendMessageSignal(controlServer->processAddress,
                                     controlServer->processPort,
                                     messageHeader(UNSUBSCRIBE_SERVICE),
                                     message);
        }
    }
    else
        DIALOGCommon::logMessage(QString("Service %1 is unknown.").arg(serviceName));
}

void Server::getListOfAvailableServicesSlot(QString serviceNameRegex)
{
    if (controlServer != NULL)
    {
        if(isConnectedToControlServer())
            Q_EMIT sendMessageSignal(controlServer->processAddress,
                                     controlServer->processPort,
                                     messageHeader(QString(LIST_OF_AVAILABLE_SERVICES)
                                                   .append(SEPARATOR)
                                                   .append(serviceNameRegex)));
    }
    else
    {
        DIALOGCommon::logMessage("The process is not connected to ControlServer.");
    }
}

void Server::registerCommandSlot(QString commandName)
{
    if(!commands.contains(commandName))
    {
        Command* command = new Command(commandName);
        commands[commandName] = command;
        command->addReceiver(serverProcess);
        serverProcess->addCommand(command);
    }

    if(isConnectedToControlServer())
    {
        QByteArray* message = new QByteArray();
        message->append(commandName);

        Q_EMIT sendMessageSignal(controlServer->processAddress,
                                 controlServer->processPort,
                                 messageHeader(REGISTER_COMMAND),
                                 message);
    }
}

void Server::unRegisterCommandSlot(QString commandName)
{
    if(commands.contains(commandName))
    {
        Command* command = commands[commandName];
        serverProcess->removeCommand(command);
        commands.remove(commandName);
        delete command;

        if(isConnectedToControlServer())
        {
            QByteArray* message = new QByteArray();
            message->append(commandName);

            Q_EMIT sendMessageSignal(controlServer->processAddress,
                                     controlServer->processPort,
                                     messageHeader(UNREGISTER_COMMAND),
                                     message);
        }
    }
    else
        DIALOGCommon::logMessage(QString("Command %1 is unknown.").arg(commandName));
}

void Server::registerProcedureSlot(QString procedureName)
{
    if(!procedures.contains(procedureName))
    {
        Procedure* procedure = new Procedure(procedureName);
        procedures[procedureName] = procedure;
        procedure->addSender(serverProcess);
        serverProcess->addProcedure(procedure);
    }
    if(isConnectedToControlServer())
    {
        QByteArray* message = new QByteArray();
        message->append(procedureName);

        Q_EMIT sendMessageSignal(controlServer->processAddress,
                                 controlServer->processPort,
                                 messageHeader(REGISTER_PROCEDURE),
                                 message);
    }
}

void Server::unRegisterProcedureSlot(QString procedureName)
{
    if(procedures.contains(procedureName))
    {
        Command* procedure = commands[procedureName];
        serverProcess->removeCommand(procedure);
        commands.remove(procedureName);
        delete procedure;

        if(isConnectedToControlServer())
        {
            QByteArray* message = new QByteArray();
            message->append(procedureName);

            Q_EMIT sendMessageSignal(controlServer->processAddress,
                                     controlServer->processPort,
                                     messageHeader(UNREGISTER_PROCEDURE),
                                     message);
        }
    }
    else
        DIALOGCommon::logMessage(QString("Procedure %1 is unknown)").arg(procedureName));

}

void Server::serverErrorSlot(const QString& error)
{
    QStringList errorList = error.split(SEPARATOR);

    Q_EMIT serverErrorSignal(error);

    if(errorList[1] == HOST_NOT_FOUND_ERROR)
    {
        lostControlServer(errorList[2] ,errorList[3].toUShort());
        DIALOGCommon::logMessage(QString("The Receiver was not found (%1, %2)")
                                 .arg(errorList[2])
                                 .arg(errorList[3]));
    }
    else if(errorList[1] == CONNECTION_REFUSED_ERROR)
    {
        lostControlServer(errorList[2] ,errorList[3].toUShort());
        DIALOGCommon::logMessage(QString("The connection was refused by the Receiver (%1, %2)")
                                 .arg(errorList[2])
                                 .arg(errorList[3]));
    }
    else
    {
        DIALOGCommon::logMessage(QString("The following error occurred in connection to"
                                         " the Receiver (%1, %2): %3.")
                                 .arg(errorList[3])
                                 .arg(errorList[4])
                                 .arg(errorList[2]));
    }
}

void Server::sendHeartBeatSlot()
{
    if(isConnectedToControlServer()) {
        Q_EMIT sendHeartBeatSignal(messageHeader(HEARTBEAT));
    }
}

void Server::checkHeartBeatSlot()
{
    QList<Message*> messages;
    quint64 now = QDateTime::currentMSecsSinceEpoch();
    bool isProcessLost = false;

    foreach(QString key, processHeartBeats.keys())
    {
        quint64 difference = now - processHeartBeats[key];
        if (difference > HEARTBEAT_CHECKER_TIMER)
        {
            processHeartBeats.remove(key);
            if(isProcessKnown(key))
            {
                Process* process = getProcess(key);
                if(process->processType != Monitoring)
                {
                    /* SENDER CRASHED */
                    foreach(Service* service, process->servicesAsSender)
                    {
                        foreach (Process* receiverProcess, service->receivers) {
                            QByteArray* lostSender = new QByteArray();
                            lostSender->append(service->serviceName);

                            Message* message = new Message(receiverProcess,
                                                           messageHeader(LOST_SENDER),
                                                           lostSender);
                            messages.append(message);
                        }
                        service->removeSender();
                    }
                    isProcessLost = true;
                }
                /* RECEIVER CRASHED */
                foreach(Service* service, process->servicesAsReceiver)
                {
                    if(service->sender != NULL)
                    {
                        QByteArray* lostReceiver = new QByteArray();
                        lostReceiver->append(process->processAddress);
                        lostReceiver->append(SEPARATOR);
                        lostReceiver->append(QString::number(process->processPort));

                        Message* message = new Message(service->sender,
                                                       messageHeader(LOST_RECEIVER),
                                                       lostReceiver);
                        messages.append(message);
                    }
                    service->removeReceiver(process);
                    if(process->processType == Custom)
                        isProcessLost = true;
                }
                /* COMMAND PROCESS CRASHED */
                foreach(Command* command, process->commands)
                {
                    command->removeReceiver(process);
                    if(process->processType == Custom)
                        isProcessLost = true;
                }
                foreach (Message* message, messages)
                {
                    if(message->receiverProcess == process)
                    {
                        messages.removeAll(message);
                        delete message->message;
                        delete message;
                    }
                }
                if (process->processType != Monitoring)
                {
                    if(process->processName == "" && process->processPID == 0)
                        DIALOGCommon::logMessage(QString("Not connected process %1 crashed or did "
                                                         "not connect to ControlServer "
                                                         "(No HeartBeats have been received).")
                                                 .arg(key));
                    else
                        DIALOGCommon::logMessage(QString("Process %1 (%2) crashed (No HeartBeats "
                                                         "have been received).")
                                                 .arg(key)
                                                 .arg(process->processName));
                }
                else
                    DIALOGCommon::logMessage(QString("Monitoring %1 crashed (No HeartBeats "
                                                     "have been received).")
                                             .arg(key));
                removeProcess(key);
            }
        }
    }
    if(isProcessLost)
        infoMonitoringSlot();

    foreach (Message* message, messages)
    {
        Q_EMIT sendMessageSignal(message->receiverProcess->processAddress,
                                 message->receiverProcess->processPort,
                                 message->header,
                                 message->message);
        messages.removeAll(message);//LEAK????
        delete message;
    }
}

void Server::infoMonitoringSlot()
{
    foreach (Process* receiverProcess, processes)
    {
        if(receiverProcess->processType != Monitoring)
            continue;

        QByteArray* info = new QByteArray();
        info->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        info->append("<processes>");
        foreach (Process* process, processes)
        {
            if(process->processType == Custom &&
               process->processName != "" && process->processPID != 0)
            {
                info->append(QString("<process name=\"%1\" address=\"%2\" port=\"%3\" pid=\"%4\">")
                             .arg(process->processName)
                             .arg(process->processAddress)
                             .arg(process->processPort)
                             .arg(process->processPID));
                foreach (Service* service, process->servicesAsSender) {
                    info->append(QString("<service name=\"%1\">")
                                 .arg(service->serviceName));
                    foreach (Process* receiver, service->receivers) {
                        if(receiver->processType == Custom)
                        {
                            info->append(QString("<receiver name=\"%1\" address=\"%2\" "
                                                 "port=\"%3\" pid=\"%4\" />")
                                         .arg(receiver->processName)
                                         .arg(receiver->processAddress)
                                         .arg(receiver->processPort)
                                         .arg(receiver->processPID));
                        }
                    }
                    info->append("</service>");
                }
                foreach (Command* command, process->commands) {
                    info->append(QString("<command name=\"%1\" />")
                                 .arg(command->commandName));
                }
                info->append("</process>");
            }
        }
        info->append("</processes>");

        Q_EMIT sendMessageSignal(receiverProcess->processAddress,
                                 receiverProcess->processPort,
                                 messageHeader(INFO_MONITORING),
                                 info);
    }
}

bool Server::hasProcessDependencies(Process* process)
{
    if (process == serverProcess)
        return true;
    foreach(Service* service, services)
    {
        if(service->sender == process)
            return true;
        foreach(Process* receiverProcess, service->receivers)
        {
            if(receiverProcess == process)
                return true;
        }
    }
    return false;
}

void Server::lostControlServer(QString errorProcessAddress, quint16 errorProcessPort)
{
    if(serverProcess->processType != ControlServer &&
       controlServer->processAddress == errorProcessAddress &&
       controlServer->processPort == errorProcessPort)
    {
        heartBeatTimer->stop();
        if(!reConnectionTimer->isActive())
        {
            foreach(Service* service, services)
            {
                Process* serviceSender = service->sender;
                if(serviceSender != NULL)
                {
                    if(serviceSender != serverProcess)
                    {
                        service->removeSender();
                        serviceSender->removeServiceAsSender(service);
                        if(!hasProcessDependencies(serviceSender))
                        {
                            removeProcess(serviceSender->processKey);
                        }
                        DIALOGCommon::logMessage(QString("Lost sender with service %1.")
                                                 .arg(service->serviceName));
                    }
                }
                foreach (Process* receiverProcess, service->receivers) {
                    if (receiverProcess != serverProcess)
                    {
                        DIALOGCommon::logMessage(QString("Lost Receiver (%1, %2)")
                                                 .arg(receiverProcess->processAddress)
                                                 .arg(receiverProcess->processPort));
                        service->removeReceiver(receiverProcess);
                        receiverProcess->removeServiceAsReceiver(service);
                        if(!hasProcessDependencies(receiverProcess))
                        {
                            removeProcess(receiverProcess->processKey);
                        }
                        break;
                    }
                }
            }
            sender->closeAllSockets();
            receiver->closeAllSockets();

            connectedToControlServer = false;
            reConnectionTimer->start(RECONNECTION_TIMER);
        }
    }
}

bool Server::isConnectedToControlServer()
{
    return connectedToControlServer;
}

bool Server::isServiceKnown(QString serviceName)
{
    return services.contains(serviceName);
}

bool Server::isCommandKnown(QString commandName)
{
    return commands.contains(commandName);
}

Sender* Server::getSender()
{
    return sender;
}

Receiver* Server::getReceiver()
{
    return receiver;
}

Process* Server::getControlServer()
{
    return controlServer;
}

Process* Server::getServerProcess()
{
    return serverProcess;
}

Service* Server::getService(QString serviceName)
{
    if(isServiceKnown(serviceName))
        return services[serviceName];
    else
        return NULL;
}

ProcessType Server::getProcessType()
{
    return processType;
}

VirtualThread* Server::getReceiverThread()
{
    return receiverThread;
}

QByteArray* Server::messageHeader(QString key)
{
    messageHeaderLock.lock();
    if(!headers.contains(key))
    {
        QByteArray* newHeader = new QByteArray();
        newHeader->append(key);
        headers[key] = newHeader;
    }
    QByteArray* header = headers[key];
    messageHeaderLock.unlock();
    return header;
}

bool Server::isProcessKnown(QString key)
{
    processLock.lock();
    bool known = processes.contains(key);
    processLock.unlock();
    return known;

}

Process* Server::getProcess(QString key)
{
    processLock.lock();
    Process* process = NULL;
    if(processes.contains(key))
        process = processes[key];
    processLock.unlock();
    return process;
}

void Server::addProcess(QString key, Process *process)
{
    processLock.lock();
    processes[key] = process;
    processLock.unlock();
}

void Server::removeProcess(QString key)
{
    processLock.lock();
    Process* process = processes[key];
    Socket* receiverSocket = process->getReceiverSocket();
    if(receiverSocket != NULL)
        receiverSocket->setProcess(NULL);
    Socket* senderSocket = process->getSenderSocket();
    if(senderSocket != NULL)
        senderSocket->setProcess(NULL);
    process->setReceiverSocket(NULL);
    process->setSenderSocket(NULL);
    sender->closeSocket(receiverSocket);
    receiver->closeSocket(senderSocket);
    processes.remove(key);
    delete process;
    processLock.unlock();
}

bool Server::isProcessConnectedToControlServer(QString key)
{
    bool connected = false;
    processLock.lock();
    if(processes.contains(key))
        connected = processes[key]->connectedToControlServer;
    processLock.unlock();
    return connected;
}

void Server::successfullyConnected()
{
    connectedToControlServer = true;

    if(reConnectionTimer->isActive())
        reConnectionTimer->stop();

    foreach(Service* service, services)
    {
        if(service->sender != NULL)
        {
            if(service->sender->processKey == serverProcess->processKey)
                registerServiceSlot(service->serviceName);
        }

        foreach(Process* receiver, service->receivers)
        {
            if(receiver->processKey == serverProcess->processKey)
                requestServiceSlot(service->serviceName);
        }
    }

    foreach(Command* command, commands)
    {
        registerCommandSlot(command->commandName);
    }

    heartBeatTimer->start(HEARTBEAT_TIMER);
    DIALOGCommon::logMessage("Successfuly connected to CommunicationControlServer.");
    emit successfullyConnectedToControlServer();
}

void Server::connectionLost()
{
    heartBeatTimer->stop();

    foreach(Service* service, services)
    {
        Process* serviceSender = service->sender;
        if(serviceSender != NULL)
        {
            if(serviceSender != serverProcess)
            {
                service->removeSender();
                serviceSender->removeServiceAsSender(service);
                if(!hasProcessDependencies(serviceSender))
                {
                    removeProcess(serviceSender->processKey);
                }

                DIALOGCommon::logMessage(QString("Lost sender with service %1.")
                                         .arg(service->serviceName));
            }
        }

        foreach (Process* receiverProcess, service->receivers) {
            if (receiverProcess != serverProcess)
            {
                DIALOGCommon::logMessage(QString("Lost Receiver (%1, %2).")
                                         .arg(receiverProcess->processAddress)
                                         .arg(receiverProcess->processPort));
                service->removeReceiver(receiverProcess);
                receiverProcess->removeServiceAsReceiver(service);
                if(!hasProcessDependencies(receiverProcess))
                {
                    removeProcess(receiverProcess->processKey);
                }
                break;
            }
        }
    }

    sender->closeAllSockets();
    receiver->closeAllSockets();

    connectedToControlServer = false;

    reConnectToControlServerSlot();
}

bool Server::waitForConnectionToControlServer(int sTimeout)
{
    if (!isConnectedToControlServer()) {
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(this, &Server::successfullyConnectedToControlServer, &loop, &QEventLoop::quit );
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit );
        timer.start(sTimeout * 1000);
        loop.exec();
    }
    return isConnectedToControlServer();
}

Server::~Server()
{
    DIALOGCommon::logMessage("Server destructor");
}
