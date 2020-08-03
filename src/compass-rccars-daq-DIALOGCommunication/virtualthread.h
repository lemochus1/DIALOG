#ifndef VIRTUALTHREAD_H
#define VIRTUALTHREAD_H

#include <QThread>
#include <QEventLoop>
#include <QDebug>
#include "process.h"

class Server;

class VirtualThread : public QThread
{
    Q_OBJECT
public:
    explicit VirtualThread();
    void setServer(Server *serverInit);
    QEventLoop* virtualThreadEventLoop;
    Server *server;

public Q_SLOTS:
    virtual void messageReceivedSlot(QString senderName, quint16 senderPort, QByteArray* header, QByteArray* message);
    virtual void serverErrorSlot(QString error);
    virtual void run();
    void startThread();
    void stop();

Q_SIGNALS:
    void sendServiceMessageSignal(QString serviceName, QByteArray* message);
    void sendCommandMessageSignal(QString commandName, QByteArray* message);
    void sendDirectCommandMessageSignal(QString commandName, QByteArray* message, QString processName);
    void sendDirectCommandUrlMessageSignal(QString commandName, QByteArray* message, QString url, int port);
    void sendProcedureCallMessageSignal(QString procedureName, QByteArray* message);
    void sendProcedureReturnMessageSignal(QString commandName, QByteArray* message, QString url, int port);

private:
    QThread* mainThread;
};

#endif // VIRTUALTHREAD_H
