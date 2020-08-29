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
    virtual void messageReceivedSlot(QString senderName,
                                     quint16 senderPort,
                                     QByteArray* header,
                                     QByteArray* message);
    virtual void serverErrorSlot(QString error);
    virtual void run();
    void startThread();
    void stop();

Q_SIGNALS:
    void sendServiceMessageSignal(QString serviceName, QByteArray* message);
    void sendServiceDataRequestSignal(QString serviceName);
    void sendServicePublisherMessageSignal(QString serviceName, QByteArray* message);
    void sendServiceSubscriberMessageSignal(QString serviceName, QByteArray* message);

    void sendCommandMessageSignal(QString commandName, QByteArray* message);
    void sendCommandNameMessageSignal(QString commandName,
                                      QByteArray* message,
                                      QString processName);
    void sendCommandAddressMessageSignal(QString commandName,
                                         QByteArray* message,
                                         QString address,
                                         int port);

    void sendProcedureCallMessageSignal(QString procedureName,
                                        QByteArray* message,
                                        int callerId);
    void sendProcedureCallNameMessageSignal(QString procedureName,
                                            QByteArray* message,
                                            QString processName,
                                            int callerId);
    void sendProcedureCallAddressMessageSignal(QString procedureName,
                                               QByteArray* message,
                                               QString address,
                                               int port,
                                               int callerId);
    void sendProcedureReturnMessageSignal(QString procedureName,
                                          QByteArray* message,
                                          QString address,
                                          int port,
                                          int callerId);
    void sendProcedureFailedMessageSignal(QString procedureName,
                                          QByteArray* message,
                                          QString address,
                                          int port,
                                          int callerId);
    void sendProcedureInvalidParamsMessageSignal(QString procedureName,
                                                 QString address,
                                                 int port,
                                                 int callerId);
private:
    QThread* mainThread;
};

#endif // VIRTUALTHREAD_H
