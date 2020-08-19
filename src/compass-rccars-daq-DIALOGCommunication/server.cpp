#include "server.h"
#include <QRegularExpression>
#include <unistd.h>

Server::Server()
{

}

Server::Server(QString serverNameInit, ProcessType processTypeInit, QString controlServerAddressInit, quint16 controlServerPortInit, VirtualThread* senderThreadInit, VirtualThread* receiverThreadInit)
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
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "ServerAddress could not be detected.";
                hardStopSlot();
            }
        }

        /*if(serverAddress.contains("pccogw")) // It should not run on GATEWAY
        {
            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "You can not run " << serverName << " on gateway (pccogwXX machine). Please, use any pccorcXX machine instead.";
            hardStopSlot();
        }*/
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

    QObject::connect(this, &Server::sendMessageSignal, sender, &Sender::sendMessageSlot, Qt::DirectConnection);
    QObject::connect(this, &Server::sendHeartBeatSignal, sender, &Sender::sendHeartBeatSlot);
    QObject::connect(sender, &Sender::senderErrorSignal, this, &Server::serverErrorSlot);

    /* TIMERS */
    if(processType != ControlServer)
    {
        connectedToControlServer = false;
        heartBeatTimer = new QTimer();
        heartBeatCheckerTimer = NULL;
        reConnectionTimer = new QTimer();
        QObject::connect(heartBeatTimer, &QTimer::timeout, this, &Server::sendHeartBeatSlot);
        QObject::connect(reConnectionTimer, &QTimer::timeout, this, &Server::reConnectToControlServerSlot);

        QObject::connect(receiverThread, &VirtualThread::started, receiver, &Receiver::startThread, Qt::DirectConnection);
        QObject::connect(receiver, &Receiver::tcpServerStartedSignal, this, &Server::receiverStartSlot);
        QObject::connect(this, &Server::receiverStartedSignal, sender, &Sender::startThread, Qt::DirectConnection);
        QObject::connect(sender, &Sender::senderStartedSignal, senderThread, &VirtualThread::startThread, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendServiceMessageSignal, sender, &Sender::sendServiceMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendCommandMessageSignal, sender, &Sender::sendCommandMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendDirectCommandMessageSignal, sender, &Sender::sendDirectCommandMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendDirectCommandUrlMessageSignal, sender, &Sender::sendDirectCommandUrlMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendProcedureCallMessageSignal, sender, &Sender::callProcedureMessageSlot, Qt::DirectConnection);
        QObject::connect(senderThread, &VirtualThread::sendProcedureReturnMessageSignal, sender, &Sender::sendProcedureReturnMessageSlot, Qt::DirectConnection);

        QObject::connect(this, &Server::serverErrorSignal, receiverThread, &VirtualThread::serverErrorSlot);

        QObject::connect(this, &Server::stopInitiated, senderThread, &VirtualThread::stop);
        QObject::connect(senderThread, &VirtualThread::finished, senderThread, &QObject::deleteLater);
        QObject::connect(senderThread, &VirtualThread::destroyed, sender, &Sender::stop);
        QObject::connect(sender, &Sender::finished, sender, &QObject::deleteLater);
        QObject::connect(sender, &Sender::destroyed, receiver, &Receiver::stop);
        QObject::connect(receiver, &Sender::finished, receiver, &QObject::deleteLater);
        QObject::connect(receiver, &Sender::destroyed, receiverThread, &VirtualThread::stop);
        QObject::connect(receiverThread, &VirtualThread::finished, receiverThread, &QObject::deleteLater);
        QObject::connect(receiverThread, &VirtualThread::destroyed, this, &Server::stopServerSlot);
        QObject::connect(this, &Server::finished, this, &Server::deleteLater);

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
        QObject::connect(heartBeatCheckerTimer, &QTimer::timeout, this, &Server::checkHeartBeatSlot);
        heartBeatCheckerTimer->start(HEARTBEAT_CHECKER_TIMER);

        QObject::connect(receiver, &Receiver::tcpServerStartedSignal, this, &Server::receiverStartSlot);
        QObject::connect(this, &Server::receiverStartedSignal, sender, &Sender::startThread, Qt::DirectConnection);

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

    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "End of Server EventLoop";

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
    serverProcess = new Process(serverAddress, serverPort, processType, serverName, QCoreApplication::applicationPid());
    addProcess(serverProcess->processKey, serverProcess);

    Q_EMIT receiverStartedSignal();
}

void Server::hardStopSlot()
{
    QString command("kill -9 ");
    command.append(QString::number(QCoreApplication::applicationPid()));

    system(command.toUtf8().data());
}

void Server::messageReceivedSlot(QString senderAddress, quint16 senderPort, QByteArray* header, QByteArray* message)
{
    QString senderKey = senderAddress + SEPARATOR + QString::number(senderPort);

    if(serverProcess->processType == ControlServer)
    {
        if(header->contains(CONNECT_TO_CONTROL_SERVER))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

            if(!isProcessConnectedToControlServer(senderKey))
            {
                Process* process = getProcess(senderKey);
                process->processType = static_cast<ProcessType>(messageList[0].toInt());
                process->processName = messageList[1];
                process->processPID = messageList[2].toULongLong();
                process->connectedToControlServer = true;
                Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(SUCCESSFULY_CONNECTED));
                if(process->processType == Custom)
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The process " << messageList[1] << " (" << senderAddress << ", " << senderPort << ") is successfully connected to CommunicationControlServer.";
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "A new Monitoring (" << senderAddress << ", " << senderPort << ") is successfully connected to CommunicationControlServer.";

                infoMonitoringSlot();
            }
            else
            {
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The process " << messageList[1] << " (" << senderAddress << ", " << senderPort << ") has been already connected to CommunicationControlServer.";
            }
            processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();
        }
        else if(isProcessKnown(senderKey) && isProcessConnectedToControlServer(senderKey))
        {
            Process* sender = getProcess(senderKey);
            if(header->contains(REGISTER_SERVICE))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(!services.contains(messageList[0]))
                {
                    Service* service = new Service(messageList[0]);
                    service->addSender(sender);
                    sender->addServiceAsSender(service);
                    services[messageList[0]] = service;
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New service " << messageList[0] << " has been registered on CommunicationControlServer.";

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
                                Q_EMIT sendMessageSignal(receiver->processAddress, receiver->processPort, messageHeader(INFO_SERVICE), infoService);
                            }
                        }

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New sender of service " << messageList[0] << " has been registered on CommunicationControlServer. Info about service " << messageList[0] << " has been sent.";

                        infoMonitoringSlot();
                    }
                    else
                    {
                        QByteArray* dialogError = new QByteArray();
                        dialogError->append(service->serviceName);

                        Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(QString(DIALOG_ERROR).append(SEPARATOR).append(SERVICE_DUPLICATE)), dialogError);

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is already registered on ControlSever.";
                    }
                }
            }
            else if(header->contains(REQUEST_SERVICE))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

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
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Info about service " << messageList[0] << " has been sent.";
                    }
                    else
                    {
                        infoService->append("unknown");
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Requested service " << messageList[0] << " has no sender on ControlSever.";
                    }
                }
                else
                {
                    Service* service = new Service(messageList[0]);
                    service->addReceiver(sender);
                    sender->addServiceAsReceiver(service);
                    services[messageList[0]] = service;

                    infoService->append("unknown");
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Requested service " << messageList[0] << " has no sender on CommunicationControlSever.";
                }

                Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(INFO_SERVICE), infoService);
                if(sender->processType != Monitoring)
                    infoMonitoringSlot();
            }
            else if(header->contains(UNSUBSCRIBE_SERVICE))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(services.contains(messageList[0]))
                {
                    Service* service = services[messageList[0]];
                    bool isSubscribed = false;
                    if(sender->servicesAsReceiver.contains(service))
                    {
                        isSubscribed = true;
                    }

                    if(isSubscribed)
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

                            Q_EMIT sendMessageSignal(service->sender->processAddress, service->sender->processPort, messageHeader(UNSUBSCRIBE_SERVICE), unsubscribeService);
                        }

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " has been unsubscribed by (" << senderAddress << ", " << senderPort << ").";

                        if(sender->processType != Monitoring)
                            infoMonitoringSlot();
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not subscribed by (" << senderAddress << ", " << senderPort << ").";
                    }
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not provided on the CommunicationControlServer.";
            }
            else if(header->contains(LIST_OF_AVAILABLE_SERVICES))
            {
                QList<QByteArray> headerList = header->split(SEPARATOR);

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
                //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "List of available services [" << numberOfAvailableServices << "] for regular expression " << headerList[1] << " has been sent.";

                Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(*header), listOfAvailableServices);
            }
            else if(header->contains(REGISTER_COMMAND) && !header->contains(UNREGISTER_COMMAND))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(!commands.contains(messageList[0]))
                {
                    Command* command = new Command(messageList[0]);
                    command->addReceiver(sender);
                    sender->addCommand(command);
                    commands[messageList[0]] = command;
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New command" << messageList[0] << " has been registered on CommunicationControlServer.";

                    infoMonitoringSlot();
                }
                else
                {
                    Command* command = commands[messageList[0]];
                    if(command->addReceiver(sender))
                    {
                        sender->addCommand(command);

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New receiver of command " << messageList[0] << " has been registered on CommunicationControlServer.";

                        if(sender->processType != Monitoring)
                            infoMonitoringSlot();
                    }
                    else
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << messageList[0] << " is already registered by (" << senderAddress << ", " << senderPort << ") on CommunicationControlSever.";
                }
            }
            else if(header->contains(UNREGISTER_COMMAND))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(commands.contains(messageList[0]))
                {
                    Command* command = commands[messageList[0]];
                    bool isRegistered = false;
                    if(sender->commands.contains(command))
                    {
                        isRegistered = true;
                    }

                    if(isRegistered)
                    {
                        sender->removeCommand(command);
                        command->removeReceiver(sender);
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << messageList[0] << " has been unregistered by (" << senderAddress << ", " << senderPort << ").";

                        if(sender->processType != Monitoring)
                            infoMonitoringSlot();
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << messageList[0] << " is not registered by (" << senderAddress << ", " << senderPort << ").";
                    }
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << messageList[0] << " is not provided on the CommunicationControlServer.";
            }
            else if(header->contains(COMMAND_MESSAGE))
            {
                QList<QByteArray> headerList = header->split(SEPARATOR);

                if(headerList[1] != DIRECT) {
                    if(commands.contains(headerList[1]))
                    {
                        Command* command = commands[headerList[1]];
                        foreach(Process* receiver, command->receivers)
                        {
                            QByteArray* newMessage = new QByteArray();
                            newMessage->append(*message);

                            Q_EMIT sendMessageSignal(receiver->processAddress, receiver->processPort, messageHeader(*header), newMessage);

                            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << headerList[1] << " has been received on CommunicationControlServer and sent to (" << receiver->processAddress << ", " << receiver->processPort << ").";
                        }
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown command " << headerList[1] << ". The command is not registered on CommunicationControlSever.";
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

                                //server->messageHeader(QString(COMMAND_MESSAGE).append(SEPARATOR).append(DIRECT).append(SEPARATOR).append(processName).append(SEPARATOR).append(commandName))
                                QString newHeader(headerList[0]);
                                newHeader.append(SEPARATOR);
                                newHeader.append(headerList[3]);

                                Q_EMIT sendMessageSignal(receiver->processAddress, receiver->processPort, messageHeader(newHeader), newMessage);

                                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << headerList[3] << " has been received on CommunicationControlServer and sent to (" << receiver->processAddress << ", " << receiver->processPort << ").";
                                send = true;
                            }
                        }
                        if (!send){
                            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Process " << headerList[2] << " does not register command " << headerList[3] << ".";
                        }
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown command " << headerList[1] << ". The command is not registered on CommunicationControlSever.";
                    }
                }
            }
            else if(header->contains(REGISTER_PROCEDURE) && !header->contains(UNREGISTER_PROCEDURE))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(!procedures.contains(messageList[0]))
                {
                    Procedure* procedure = new Procedure(messageList[0]);
                    procedure->addSender(sender);
                    sender->addProcedure(procedure);
                    procedures[messageList[0]] = procedure;
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New procedure" << messageList[0] << " has been registered on CommunicationControlServer.";

                    infoMonitoringSlot();
                }
                else
                {//vice senderu...
//                    Command* command = commands[messageList[0]];
//                    if(command->addReceiver(sender))
//                    {
//                        sender->addCommand(command);

//                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "New receiver of command " << messageList[0] << " has been registered on CommunicationControlServer.";

//                        if(sender->processType != Monitoring)
//                            infoMonitoringSlot();
//                    }
//                    else
//                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << messageList[0] << " is already registered by (" << senderAddress << ", " << senderPort << ") on CommunicationControlSever.";
                }
            }
            else if(header->contains(UNREGISTER_PROCEDURE))
            {
                QList<QByteArray> messageList = message->split(SEPARATOR);

                if(procedures.contains(messageList[0]))
                {
                    Procedure* procedure = procedures[messageList[0]];
                    bool isRegistered = false;
                    if(sender->procedures.contains(procedure))
                    {
                        isRegistered = true;
                    }

                    if(isRegistered)
                    {
                        sender->removeProcedure(procedure);

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Procedure " << messageList[0] << " has been unregistered by (" << senderAddress << ", " << senderPort << ").";

                        if(sender->processType != Monitoring)
                            infoMonitoringSlot();
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Procedure " << messageList[0] << " is not registered by (" << senderAddress << ", " << senderPort << ").";
                    }
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Procedure " << messageList[0] << " is not provided on the CommunicationControlServer.";
            }
            else if(header->contains(PROCEDURE_MESSAGE))
            {
                QList<QByteArray> headerList = header->split(SEPARATOR);

                if(headerList[1] == PROCEDURE_CALL) {

                    if(procedures.contains(headerList[2]))
                    {
                        Procedure* procedure = procedures[headerList[2]];
                        Process* receiver = procedure->sender;

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

                        Q_EMIT sendMessageSignal(receiver->processAddress, receiver->processPort, messageHeader(newHeader), newMessage);

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Procedure " << headerList[2] << " has been received on CommunicationControlServer and sent to (" << receiver->processAddress << ", " << receiver->processPort << ").";

                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown procedure " << headerList[2] << ". The procedure is not registered on CommunicationControlSever.";
                    }
                }
            }
            else if(header->contains(HEARTBEAT))
            {
                processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();
                //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "HeartBeat received from (" << senderAddress << ", " << senderPort << ").";
            }
            else
            {
                if(message == NULL)
                {
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown message from (" << senderAddress << ", " << senderPort << "): " << *header << ".";
                }
                else
                {
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown message from (" << senderAddress << ", " << senderPort << "): " << *header << *message << ".";
                }
            }
        }
        else
        {
            Q_EMIT sendMessageSignal(senderAddress, senderPort, messageHeader(CONNECTION_LOST));

            processHeartBeats[senderKey] = QDateTime::currentMSecsSinceEpoch();

            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown sender (" << senderAddress << ", " << senderPort << "). The sender is not connected to CommunicationControlServer.";
        }
    }
    else
    {
        if(header->contains(SUCCESSFULY_CONNECTED))
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
            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Successfuly connected to CommunicationControlServer.";
            emit successfullyConnectedToControlServer();
        }
        else if(header->contains(CONNECTION_LOST))
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

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost sender with service " << service->serviceName << ".";
                    }
                }

                foreach (Process* receiverProcess, service->receivers) {
                    if (receiverProcess != serverProcess)
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost Receiver (" << receiverProcess->processAddress << ", " << receiverProcess->processPort << ").";
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
        else if(header->contains(INFO_SERVICE))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

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
                            Process* senderProcess = new Process(messageList[1], messageList[2].toUShort(), Custom);

                            addProcess(senderProcessKey, senderProcess);
                            service->addSender(senderProcess);
                            senderProcess->addServiceAsSender(service);
                        }

                        subscribeServiceSlot(messageList[0]);
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "CommunicationControlServer does not know the sender of service " << messageList[0] << ".";
                    }
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is already known.";
            }
            else
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not known on the server.";
        }
        else if(header->contains(SUBSCRIBE_SERVICE) && !header->contains(UNSUBSCRIBE_SERVICE))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

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
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " has been subscribed by (" << senderAddress << ", " << senderPort << ").";
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " has already been subscribed by (" << senderAddress << ", " << senderPort << ").";
            }
            else
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not provided on the server.";
        }
        else if(header->contains(UNSUBSCRIBE_SERVICE))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

            if(services.contains(messageList[0]))
            {
                Service* service = services[messageList[0]];
                QString receiverKey = messageList[1] + SEPARATOR + messageList[2];
                if(isProcessKnown(receiverKey))
                {
                    Process* receiverProcess = getProcess(receiverKey);
                    bool isSender = false;
                    if(service->sender == serverProcess)
                    {
                        isSender = true;
                    }

                    if(isSender)
                    {
                        receiverProcess->removeServiceAsReceiver(service);
                        service->removeReceiver(receiverProcess);
                        if(!hasProcessDependencies(receiverProcess))
                        {
                            removeProcess(receiverProcess->processKey);
                        }

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " has been unsubscribed by (" << messageList[1] << ", " << messageList[2] << ").";
                    }
                    else
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "This process (" << serverProcess->processAddress << ", " << serverProcess->processPort << ") is not the sender for Service " << messageList[0] << ".";
                    }
                }
                else
                {
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Process (" << senderAddress << ", " << senderPort << ") is not known on the server.";
                }
            }
            else
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not provided on the server.";
        }
        else if(header->contains(LOST_SENDER))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

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

                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost sender with service " << messageList[0] << ".";
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The sender with service " << messageList[0] << " has not been already known. It could not be deleted.";
            }
            else
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << messageList[0] << " is not known on the server.";
        }
        else if(header->contains(LOST_RECEIVER))
        {
            QList<QByteArray> messageList = message->split(SEPARATOR);

            foreach (Service* service, services)
            {
                foreach (Process* receiverProcess, service->receivers)
                {
                    if (receiverProcess->processAddress == messageList[0] && receiverProcess->processPort == messageList[1].toUShort())
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost Receiver (" << receiverProcess->processAddress << ", " << receiverProcess->processPort << ").";
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
        else
        {
            if(message == NULL)
            {
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown message from (" << senderAddress << ", " << senderPort << "): " << *header << ".";
            }
            else
            {
                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Unknown message from (" << senderAddress << ", " << senderPort << "): " << *header << *message << ".";
            }
        }
    }

    delete header;
    delete message;
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

        Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(CONNECT_TO_CONTROL_SERVER), message);

        waitForConnectionToControlServer();
        if (!isConnectedToControlServer()) {
            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " Connection to control server failed. ";
        }

    }
    else
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "ControlServer is trying to connect to itself. It does not make any sense.";
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

        Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(REGISTER_SERVICE), message);
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

        Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(REQUEST_SERVICE), message);
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

            Q_EMIT sendMessageSignal(senderProcess->processAddress, senderProcess->processPort, messageHeader(SUBSCRIBE_SERVICE), message);
        }
    }
    else
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << serviceName << " is unknown.";
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

            Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(UNSUBSCRIBE_SERVICE), message);
        }
    }
    else
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Service " << serviceName << " is unknown.";
}

void Server::getListOfAvailableServicesSlot(QString serviceNameRegex)
{
    if (controlServer != NULL)
    {
        if(isConnectedToControlServer())
            Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(QString(LIST_OF_AVAILABLE_SERVICES).append(SEPARATOR).append(serviceNameRegex)));
    }
    else
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The process is not connected to ControlServer.";
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

        Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(REGISTER_COMMAND), message);
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

            Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(UNREGISTER_COMMAND), message);
        }
    }
    else
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Command " << commandName << " is unknown.";
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

        Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(REGISTER_PROCEDURE), message);
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

            Q_EMIT sendMessageSignal(controlServer->processAddress, controlServer->processPort, messageHeader(UNREGISTER_PROCEDURE), message);
        }
    }
    else
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Procedure " << procedureName << " is unknown.";

}

void Server::serverErrorSlot(QString error)
{
    QStringList errorList = error.split(SEPARATOR);

    Q_EMIT serverErrorSignal(error);

    if(errorList[1] == HOST_NOT_FOUND_ERROR)
    {
        lostControlServer(errorList[2] ,errorList[3].toUShort());
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The Receiver was not found (" << errorList[2] << ", " << errorList[3] << ").";
    }
    else if(errorList[1] == CONNECTION_REFUSED_ERROR)
    {
        lostControlServer(errorList[2] ,errorList[3].toUShort());
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The connection was refused by the Receiver (" << errorList[2] << ", " << errorList[3] << ").";
    }
    else
    {
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "The following error occurred in connection to the Receiver (" << errorList[3] << ", " << errorList[4] << "): " << errorList[2];
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

                            Message* message = new Message(receiverProcess, messageHeader(LOST_SENDER), lostSender);
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

                        Message* message = new Message(service->sender, messageHeader(LOST_RECEIVER), lostReceiver);
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
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Not connected process " << key << " crashed or did not connect to ControlServer (No HeartBeats have been received).";
                    else
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Process " << key << " (" << process->processName << ") crashed (No HeartBeats have been received).";
                }
                else
                    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Monitoring " << key << " crashed (No HeartBeats have been received).";

                removeProcess(key);
            }
        }
    }

    if(isProcessLost)
        infoMonitoringSlot();

    foreach (Message* message, messages)
    {
        Q_EMIT sendMessageSignal(message->receiverProcess->processAddress, message->receiverProcess->processPort, message->header, message->message);
        messages.removeAll(message);
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
            if(process->processType == Custom && process->processName != "" && process->processPID != 0)
            {
                info->append(QString("<process name=\"%1\" address=\"%2\" port=\"%3\" pid=\"%4\">").arg(process->processName).arg(process->processAddress).arg(process->processPort).arg(process->processPID));
                foreach (Service* service, process->servicesAsSender) {
                    info->append(QString("<service name=\"%1\">").arg(service->serviceName));
                    foreach (Process* receiver, service->receivers) {
                        if(receiver->processType == Custom)
                        {
                            info->append(QString("<receiver name=\"%1\" address=\"%2\" port=\"%3\" pid=\"%4\" />").arg(receiver->processName).arg(receiver->processAddress).arg(receiver->processPort).arg(receiver->processPID));
                        }
                    }
                    info->append("</service>");
                }
                foreach (Command* command, process->commands) {
                    info->append(QString("<command name=\"%1\" />").arg(command->commandName));
                }
                info->append("</process>");
            }
        }
        info->append("</processes>");

        Q_EMIT sendMessageSignal(receiverProcess->processAddress, receiverProcess->processPort, messageHeader(INFO_MONITORING), info);
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
    if(serverProcess->processType != ControlServer && controlServer->processAddress == errorProcessAddress && controlServer->processPort == errorProcessPort)
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

                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost sender with service " << service->serviceName << ".";
                    }
                }

                foreach (Process* receiverProcess, service->receivers) {
                    if (receiverProcess != serverProcess)
                    {
                        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Lost Receiver (" << receiverProcess->processAddress << ", " << receiverProcess->processPort << ").";
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
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " " << "Server destructor";
}
