#ifndef PROCESS_H
#define PROCESS_H

#include <QMutex>
#include <QObject>
#include "define.h"
#include "server.h"
#include "messagecontainer.h"

class Service;
class Command;
class Socket;
class Procedure;

class Process
{
public:
    explicit Process(QString processAddressInit, quint16 processPortInit, ProcessType processTypeInit = Custom, QString processNameInit = "", quint64 processPIDInit = 0);
    ~Process();
    // server a az api
    bool addServiceAsSender(Service* service);
    void removeServiceAsSender(Service* service);
    bool addServiceAsReceiver(Service* service);
    void removeServiceAsReceiver(Service* service);
    bool addCommand(Command* command);
    void removeCommand(Command* command);
    bool sendMessage(MessageContainer* messageContainer);
    void setReceiverSocket(Socket* socket);
    Socket* getReceiverSocket();
    void setSenderSocket(Socket* socket);
    Socket* getSenderSocket();
    bool addProcedure(Procedure* procedure);
    void removeProcedure(Procedure* procedure);

    QString processAddress;
    quint16 processPort;
    ProcessType processType;
    QString processName;
    quint64 processPID;
    QString processKey;
    QList<Service*> servicesAsSender;
    QList<Service*> servicesAsReceiver;
    QList<Command*> commands;
    QList<Procedure*> procedures;
    Socket* receiverSocket;
    Socket* senderSocket;
    QMutex socketLock;
    bool connectedToControlServer;
};

#endif // PROCESS_H
