#ifndef DIALOGAPI_H
#define DIALOGAPI_H
#include <QObject>
#include "server.h"
#include "define.h"

class DIALOGServicePublisher;
class DIALOGServiceSubscriber;
class DIALOGData;
class DIALOGCommand;
class DIALOGProcedureCaller;
class DIALOGProcedureHandler;

class DIALOGProcess : public QObject
{
    Q_OBJECT

    class ReceiverThread;
    class SenderThread;    

public:
    explicit DIALOGProcess(QString nameInit, QObject* parent=nullptr);
    virtual ~DIALOGProcess();

    void start(QThread::Priority priority = QThread::NormalPriority);
    void setControlServerAdress(QString url, int port);

    QString getName() const;
    
    void registerCommand(DIALOGCommand* command);
    DIALOGCommand* registerCommand(QString name);

    void registerProcedure(DIALOGProcedureHandler* procedure);
    DIALOGProcedureHandler* registerProcedure(QString name);

    void registerService(DIALOGServicePublisher* publisher);
    DIALOGServicePublisher* registerService(QString name);

    void requestService(DIALOGServiceSubscriber* subscriber);
    DIALOGServiceSubscriber* requestService(QString name);

    DIALOGProcedureCaller* callProcedure(QString name, QByteArray message);
    //DIALOGProcedureCaller* callProcedure(QString name, QString process);
    //DIALOGProcedureCaller* callProcedure(QString name, QString url, int port);

public Q_SLOTS:

//    void registerProcedureSlot(DIALOGProcedureHandler* handler);
//    DIALOGProcedureHandler registerProcedureSlot(QString name);
    
    void sendCommandSlot(QString name, QByteArray message);
    void sendDirectCommandSlot(QString name, QByteArray message, QString processName);
    void sendDirectCommandSlot(QString name, QByteArray message, QString url, int port);

    void stopSlot();
    
Q_SIGNALS:
    void stoppedSignal();
    void serverDestroyed();

private:
    Server* server;
    SenderThread* sender;
    ReceiverThread* receiver;

    QString name;
    QString controlServerUrl;
    int controlServerPort;
};


class DIALOGProcess::ReceiverThread : public VirtualThread 
{
    Q_OBJECT

public:
    ~ReceiverThread();
    void run() override;

    void registerCommand(DIALOGCommand* command);
    void registerService(DIALOGServiceSubscriber* subscriber);
    void registerProcedureCaller(DIALOGProcedureCaller* caller);
    void registerProcedureHandler(DIALOGProcedureHandler* handler);

public slots:
    void messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray *header, QByteArray *message) override;     

private:
    QMap<QString, DIALOGCommand*> commands;
    QMap<QString, DIALOGServiceSubscriber*> subscribers;
    QMap<QString, DIALOGProcedureCaller*> procedureCalls;
    QMap<QString, DIALOGProcedureHandler*> procedureHandlers;
};


class DIALOGProcess::SenderThread : public VirtualThread
{
    Q_OBJECT

public:
    ~SenderThread();
    void run() override;

    void addServiceToRequest(QString name);
    void addServiceToRegister(DIALOGServicePublisher* publisher);
    void addCommandToRegister(QString name);
    void addProcedureToRegister(DIALOGProcedureHandler* handler);

public slots:
    void sendCommandSlot(QString name, QByteArray* message);
    void sendDirectCommandSlot(QString name, QByteArray* message, QString processName);
    void sendDirectCommandUrlSlot(QString name, QByteArray* message, QString url, int port);
    void sendServiceSlot(QString name, QByteArray message);
    void callProcedureSlot(QString name, QByteArray message);
    void sendProcedureReturnSlot(QString name, QByteArray data, QString url, int port);

private:
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
    explicit DIALOGServicePublisher(QString nameInit, QObject* parent=nullptr): DIALOGData(nameInit, parent) {}
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
    explicit DIALOGServiceSubscriber(QString nameInit, QObject* parent=nullptr): DIALOGData(nameInit, parent) {}
    virtual ~DIALOGServiceSubscriber() {}

public Q_SLOTS:
    virtual void dataUpdatedSlot(QByteArray dataInit);
//    void sendDirectCommand(QString name);

Q_SIGNALS:
    void dataUpdatedSignal(QByteArray data);
};

class DIALOGCommand : public QObject
{
    Q_OBJECT
    
public:
    DIALOGCommand(QString nameInit) {name = nameInit;}

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
    explicit DIALOGProcedureCaller(QString nameInit, QObject* parent=nullptr): QObject(parent) {name = nameInit;}
    virtual ~DIALOGProcedureCaller() {}

    QByteArray waitForData(bool &ok, int timeout=10);

    QByteArray tryGetData();

    QString  getName() const;

public Q_SLOTS:
    void setDataSlot(QByteArray dataInit);


Q_SIGNALS:
    void dataSetSignal();
};

class DIALOGProcedureHandler : public QObject
{
    Q_OBJECT

    QString name;
    QMutex mutex;
    QByteArray data;

    QString url;
    int port;
    
public:
    explicit DIALOGProcedureHandler(QString nameInit, QObject* parent=nullptr): QObject(parent) {name = nameInit;}
    virtual ~DIALOGProcedureHandler() {}

    QString  getName() const;

public Q_SLOTS:
    virtual void callRequestedSlot(QByteArray params, QString urlInit, int portInit);
    void callFinishedSlot(QByteArray dataInit);
   
Q_SIGNALS:
    void callRequestedSignal(QByteArray params);
    void callFinishedSignal(QString name, QByteArray dataInit, QString url, int port);
};

#endif // DIALOGAPI_H
