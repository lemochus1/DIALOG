#ifndef DIALOGAPI_H
#define DIALOGAPI_H

#include <QObject>
#include <QHostInfo>

#include "server.h"
#include "define.h"
#include "processaddress.h"

class DIALOGServicePublisher;
class DIALOGServiceSubscriber;
class DIALOGData;
class DIALOGCommandHandler;
class DIALOGProcedureCaller;
class DIALOGProcedurePublisher;

class DIALOGProcess
{
    class ReceiverThread;
    class SenderThread;

    enum {NotConnectedError};

public:
    static DIALOGProcess &GetInstance();

    void start(QThread::Priority priority = QThread::NormalPriority);
    void stop();

    void setControlServerAddress(const QString& address, quint16 port);

    bool setName(const QString& nameInit);
    QString getName() const;

    bool registerCommand(QWeakPointer<DIALOGCommandHandler> command);
    QSharedPointer<DIALOGCommandHandler> registerCommand(const QString& name);

    bool registerProcedure(QWeakPointer<DIALOGProcedurePublisher> procedure);
    QSharedPointer<DIALOGProcedurePublisher> registerProcedure(const QString& name);

    bool registerService(QWeakPointer<DIALOGServicePublisher> publisher);
    QSharedPointer<DIALOGServicePublisher> registerService(const QString& name);

    bool requestService(QWeakPointer<DIALOGServiceSubscriber> subscriber);
    QSharedPointer<DIALOGServiceSubscriber> requestService(const QString& name);

    QSharedPointer<DIALOGProcedureCaller> callProcedure(const QString& name, const QByteArray& params);
    //DIALOGProcedureCaller* callProcedure(QString name, QString process);
    //DIALOGProcedureCaller* callProcedure(QString name, QString url, int port);

    bool sendCommand(const QString& name, const QByteArray& message);
    bool sendDirectCommand(const QString& name,
                           const QByteArray& message,
                           const QString& processName);

    bool sendDirectCommand(const QString& name,
                           const QByteArray& message,
                           const QString& address,
                           quint16 port);

private:
    DIALOGProcess();
    ~DIALOGProcess();

    static DIALOGProcess* instance;

    template<typename T>
    bool handleMessageTypeImpl(QWeakPointer<T> handler, DIALOGMessageHandlerType type);

    template<typename T>
    QSharedPointer<T> handleMessageTypeImpl(const QString& name, DIALOGMessageHandlerType type);

public:
    DIALOGProcess(DIALOGProcess const&) = delete;
    void operator=(DIALOGProcess const&) = delete;

private:
    QString name;

    QPointer<Server> server;
    QPointer<SenderThread> sender;
    QPointer<ReceiverThread> receiver;

    ProcessAddress controlAddress;
};


class DIALOGProcess::ReceiverThread : public VirtualThread 
{
    Q_OBJECT

public:
    ~ReceiverThread();
    void run() override;

    bool registerMessageHandler(QWeakPointer<DIALOGCommandHandler> handler);
    bool registerMessageHandler(QWeakPointer<DIALOGServiceSubscriber> handler);
    bool registerMessageHandler(QWeakPointer<DIALOGServicePublisher> handler);
    bool registerMessageHandler(QWeakPointer<DIALOGProcedureCaller> handler);
    bool registerMessageHandler(QWeakPointer<DIALOGProcedurePublisher> handler);

public slots:
    void messageReceivedSlot(QString senderName,
                             quint16 senderPort,
                             QByteArray *header,
                             QByteArray *message) override;

private:
    template<typename T>
    bool registerMessageHandlerImpl(QWeakPointer<T> handler,
                                    QMap<QString, QWeakPointer<T>>& map,
                                    DIALOGMessageHandlerType type);

    template<typename T>
    bool invokeMethod(QWeakPointer<T> handler, const char *method, QByteArray * message);

    QMap<QString, QWeakPointer<DIALOGCommandHandler>> commands;
    QMap<QString, QWeakPointer<DIALOGServiceSubscriber>> serviceSubscribers;
    QMap<QString, QWeakPointer<DIALOGProcedureCaller>> procedureCallers;
    QMap<QString, QWeakPointer<DIALOGProcedurePublisher>> procedurePublishers;
};


class DIALOGProcess::SenderThread : public VirtualThread
{
    Q_OBJECT

public:
    ~SenderThread();
    void run() override;

    bool registerMessageSender(QWeakPointer<DIALOGCommandHandler> sender);
    bool registerMessageSender(QWeakPointer<DIALOGServiceSubscriber> sender);
    bool registerMessageSender(QWeakPointer<DIALOGServicePublisher> sender);
    bool registerMessageSender(QWeakPointer<DIALOGProcedureCaller> sender);
    bool registerMessageSender(QWeakPointer<DIALOGProcedurePublisher> sender);

public slots:
    void sendCommandSlot(QString name, QByteArray* message);
    void sendDirectCommandSlot(QString name, QByteArray* message, QString processName);
    void sendDirectCommandUrlSlot(QString name, QByteArray* message, QString address, quint16 port);
    void sendServiceSlot(QString name, QByteArray message);
    void callProcedureSlot(QString name, QByteArray message);
    void sendProcedureReturnSlot(QString name, QByteArray data, QString address, int port);

private:
    template<typename T>
    bool registerMessageSenderImpl(QWeakPointer<T> sender,
                                   QStringList& list,
                                   DIALOGMessageHandlerType type);

    QStringList servicesToRegister;
    QStringList servicesToRequest;
    QStringList commandsToRegister;
    QStringList proceduresToRegister;
};


class DIALOGData : public QObject
{
    Q_OBJECT
    
public:
    explicit DIALOGData(QString nameInit, QObject* parent=nullptr): QObject(parent) {name = nameInit;}
    virtual ~DIALOGData() {}

    QByteArray getData() const {return data;}
    QString getName() const {return name;}

protected:
    QByteArray data;
    QString name;    

};

class DIALOGServicePublisher : public DIALOGData
{
    Q_OBJECT
    
public:
    explicit DIALOGServicePublisher(QString nameInit, QObject* parent=nullptr)
        : DIALOGData(nameInit, parent) {}
    virtual ~DIALOGServicePublisher() {}

public Q_SLOTS:
    void updateDataSlot(QByteArray dataInit);
//    void dataUpdateRequestedSlot();

Q_SIGNALS:
    void dataUpdatedSignal(QString name, QByteArray data);
//    void dataUpdateRequestedSignal();
};

class DIALOGServiceSubscriber : public DIALOGData
{
    Q_OBJECT
    
public:
    explicit DIALOGServiceSubscriber(QString nameInit, QObject* parent=nullptr)
        : DIALOGData(nameInit, parent) {}
    virtual ~DIALOGServiceSubscriber() {}

public Q_SLOTS:
    virtual void dataUpdatedSlot(QByteArray dataInit);
//    void sendDirectCommand(QString name);

Q_SIGNALS:
    void dataUpdatedSignal(QByteArray data);
};

class DIALOGCommandHandler : public QObject
{
    Q_OBJECT
    
public:
    DIALOGCommandHandler(QString nameInit) {name = nameInit;}

    QString getName(){return name;}

public Q_SLOTS:
    virtual void commandReceivedSlot(QByteArray message);

Q_SIGNALS:
    void commandReceivedSignal(QByteArray message);

private:
    QString name;
};


class DIALOGProcedureCaller : public QObject
{
    Q_OBJECT
    
    QString name;
    QMutex mutex;
    QByteArray data;
    bool dataSet  = false;

public:
    explicit DIALOGProcedureCaller(QString nameInit, QObject* parent=nullptr)
        : QObject(parent) {name = nameInit;}
    virtual ~DIALOGProcedureCaller() {}

    QByteArray waitForData(bool &ok, int timeout=10);

    QByteArray tryGetData();

    QString  getName() const;

public Q_SLOTS:
    void setDataSlot(QByteArray dataInit);


Q_SIGNALS:
    void dataSetSignal();
};

class DIALOGProcedurePublisher : public QObject
{
    Q_OBJECT

    QString name;
    QMutex mutex;
    QByteArray data;

    QString url;
    int port;
    
public:
    explicit DIALOGProcedurePublisher(QString nameInit, QObject* parent=nullptr)
        : QObject(parent) {name = nameInit;}
    virtual ~DIALOGProcedurePublisher() {}

    QString  getName() const;

public Q_SLOTS:
    virtual void callRequestedSlot(QByteArray params, QString urlInit, int portInit);
    void callFinishedSlot(QByteArray dataInit);
   
Q_SIGNALS:
    void callRequestedSignal(QByteArray params);
    void callFinishedSignal(QString name, QByteArray dataInit, QString url, int port);
};

#endif // DIALOGAPI_H
