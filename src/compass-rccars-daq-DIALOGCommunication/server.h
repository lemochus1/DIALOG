#ifndef SERVER_H
#define SERVER_H

#include "sender.h"
#include "receiver.h"
#include "process.h"
#include "socket.h"
#include "service.h"
#include "define.h"
#include "message.h"
#include "command.h"
#include "procedure.h"
#include "virtualthread.h"

class Sender;
class Receiver;
class Service;
class Process;
class Socket;
class Procedure;
class Command;
class VirtualThread;

class Server : public QThread
{
    Q_OBJECT

    bool hasProcessDependencies(Process* process);
    void lostControlServer(QString errorProcessAddress, quint16 errorProcessPort);
    bool isProcessConnectedToControlServer(QString key);

//  Custom Server
    void successfullyConnected();
    void connectionLost();
    void infoService(const QStringList &messageList);
    void subscribeService(const QString& senderAddress,
                          quint16 senderPort,
                          const QStringList &messageList);
    void unsubscribeService(const QStringList &messageList);
    void lostSender(const QStringList &messageList);
    void lostReceiver(const QStringList &messageList);

//  Control Server
    void connectRequest(const QString& senderAddress,
                        quint16 senderPort,
                        const QString& senderKey,
                        const QStringList &messageList);
    void registerService(const QString& senderAddress,
                         quint16 senderPort,
                         Process *sender,
                         const QStringList &messageList);
    void requestService(const QString& senderAddress,
                        quint16 senderPort,
                        Process *sender,
                        const QStringList &messageList);
    void unsubscribeService(const QString& senderAddress,
                            quint16 senderPort,
                            Process *sender,
                            const QStringList &messageList);
    void listOfServices(const QString& senderAddress,
                        quint16 senderPort,
                        const QStringList headerList);
    void registerCommand(const QString& senderAddress,
                         quint16 senderPort,
                         Process *sender,
                         const QStringList &messageList);
    void unregisterCommand(const QString& senderAddress,
                           quint16 senderPort,
                           Process *sender,
                           const QStringList &messageList);
    void commandMessage(const QStringList &headerList, QByteArray* message);
    void registerProcesure(const QString& senderAddress,
                           quint16 senderPort,
                           Process *sender,
                           const QStringList &messageList);
    void unregisterProcedure(const QString& senderAddress,
                             quint16 senderPort,
                             Process *sender,
                             const QStringList &messageList);
    void procedureMessage(const QString& senderAddress,
                          quint16 senderPort,
                          const QStringList &headerList,
                          QByteArray* message);
    void unknownMessage(const QString& senderAddress,
                        quint16 senderPort,
                        QByteArray* header,
                        QByteArray* message);

public:
    Server(){}
    Server(QString serverNameInit,
           ProcessType processTypeInit,
           QString controlServerAddressInit,
           quint16 controlServerPortInit,
           VirtualThread* senderThreadInit = nullptr,
           VirtualThread* receiverThreadInit = nullptr);
    ~Server();
    void run();
    void stop();//Asi nikde
    bool isServiceKnown(QString serviceName);//i v ostatnich projektech
    bool isCommandKnown(QString commandName);
    Sender* getSender();//jenom v socketu pro vlastni odstraneni...
    Receiver* getReceiver();//jenom v socketu pro vlastni pridani...
    Process* getControlServer();//casto a ruzne
    Process* getServerProcess();//pouze v socket writeMessageSlot...
    Service* getService(QString serviceName);//asi nikde v projektu...
    ProcessType getProcessType();//pouze v sockets... connect to receiver thread if not control server...
    VirtualThread* getReceiverThread();//pouze tam co dito...
    QByteArray* messageHeader(QString key);//sender pouze...
    bool isProcessKnown(QString key);//jendou v socketu...
    Process* getProcess(QString key);// sender a socket...

    Process* getProcess(const QString& address, int port, bool addIfMissing = true);

    void removeReceiverSocket(Process* process, Socket *socket);

    void addProcess(QString key, Process* process);// casto...
    void removeProcess(QString key);

    void setControlServerAddress(const QString address, quint16 port);

    QMutex processLock;// casto se pouziva v socketu...

    /// Waits for signal successfullyConnectedToControlServer or timeout
    bool waitForConnectionToControlServer(int sTimeout = 10);
    bool isConnectedToControlServer();

public Q_SLOTS:
    //Napojuje se na to socket...
    void messageReceivedSlot(QString senderAddress,
                             quint16 senderPort,
                             QByteArray* header,
                             QByteArray* message = nullptr);
    //Vola se z venku api v sender Processor thread
    void connectToControlServerSlot();
    //asi muze byt private
    void reConnectToControlServerSlot();
    //z venku v threadech sender apod...
    void registerServiceSlot(QString serviceName);
    void requestServiceSlot(QString serviceName);
    void subscribeServiceSlot(QString serviceName);
    void unSubscribeServiceSlot(QString serviceName);

    //asi klidne private... i v masteru
    void getListOfAvailableServicesSlot(QString serviceNameRegex);
    //i venku z api
    void registerCommandSlot(QString commandName);
    void unRegisterCommandSlot(QString commandName);

    void registerProcedureSlot(QString procedureName);
    void unRegisterProcedureSlot(QString procedureName);


    //asi klidne private
    void serverErrorSlot(const QString &error);
    // asi klidne private
    void sendHeartBeatSlot();
    //asi klidne private
    void checkHeartBeatSlot();
    //private
    void infoMonitoringSlot();
    void stopSlot();
    //nikde
    void stopServerSlot();
    //private
    void receiverStartSlot();
    void hardStopSlot();

Q_SIGNALS:
    void sendMessageSignal(QString receiverAddress,
                           quint16 receiverPort,
                           QByteArray* header,
                           QByteArray* message = nullptr);
    void sendHeartBeatSignal(QByteArray* message);
    void serverErrorSignal(QString error);
    void receiverStartedSignal();
    void stopInitiated();

    /// Emited when confirmation messages is received.
    ///
    /// connectToControlServer waits for it.
    void successfullyConnectedToControlServer();

private:
    Process* serverProcess;
    QEventLoop* serverEventLoop;

    QThread* mainThread;
    VirtualThread* senderThread;
    VirtualThread* receiverThread;

    QMap<QString, Process*> processes;
    QMap<QString, Service*> services;
    QMap<QString, Command*> commands;
    QMap<QString, Procedure*> procedures;

    Sender* sender;
    Receiver* receiver;

    Process* controlServer;
    QString controlServerAddress;
    quint16 controlServerPort;
    bool connectedToControlServer;
    QTimer* heartBeatTimer;
    QTimer* heartBeatCheckerTimer;
    QTimer* reConnectionTimer;

    QMap<QString, quint64> processHeartBeats;

    QMap<QString, QByteArray*> headers;
    QMutex messageHeaderLock;



    /*INIT VALUES */
    ProcessType processType;
    QString serverName;
    QString serverAddress;
    quint16 serverPort;
};

#endif // SERVER_H
