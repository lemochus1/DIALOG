#ifndef SENDER_H
#define SENDER_H

#include <QtNetwork>
#include <QTcpSocket>
#include "server.h"
#include "socket.h"
#include "virtualthread.h"

class MessageContainer;
class Socket;

class Sender : public QThread
{
    Q_OBJECT
    void sendToHost(QString receiverAddress,
                    quint16 receiverPort,
                    MessageContainer* messageContainer);

public:
    Sender(Server* serverInit);
    ~Sender();

    bool anyOpenSockets();
    void closeAllSockets();
    void closeSocket(Socket* socket);
    void addSocket(Socket* socket);
    void removeSocket(Socket* socket);
    bool senderStarted;

public Q_SLOTS:
    void run();
    void sendMessageSlot(QString receiverAddress,
                         quint16 receiverPort,
                         QByteArray* header,
                         QByteArray* message = nullptr);
    void sendHeartBeatSlot(QByteArray* header);

    void sendServiceMessageSlot(QString serviceName, QByteArray* message);
    void sendServiceDataRequestMessageSlot(QString serviceName);
    void sendServicePublisherMessageSlot(QString serviceName, QByteArray* message);
    void sendServiceSubscriberMessageSlot(QString serviceName, QByteArray* message);

    void sendCommandMessageSlot(QString commandName, QByteArray* message);
    void sendCommandNameMessageSlot(QString commandName,
                                    QByteArray* message,
                                    QString processName);
    void sendCommandAddressMessageSlot(QString commandName,
                                       QByteArray* message,
                                       QString address,
                                       int port);

    void callProcedureMessageSlot(QString procedureName, QByteArray* message, int callerId);
    void callProcedureNameMessageSlot(QString procedureName,
                                      QByteArray* message,
                                      QString processName,
                                      int callerId);
    void callProcedureAddressMessageSlot(QString procedureName,
                                         QByteArray* message,
                                         QString address,
                                         int port,
                                         int callerId);
    void sendProcedureReturnMessageSlot(QString procedureName,
                                        QByteArray* message,
                                        QString address,
                                        int port,
                                        int callerId);
    void sendProcedureFailedMessageSlot(QString procedureName,
                                        QByteArray* message,
                                        QString address,
                                        int port,
                                        int callerId);
    void sendProcedureInvalidParamsMessageSlot(QString procedureName,
                                               QString address,
                                               int port,
                                               int callerId);

    void senderErrorSlot(QString error);
    void socketDisconnectedSlot();
    void startThread();
    void stop();

Q_SIGNALS:
    void connectToHostSignal();
    void senderErrorSignal(QString error);
    void senderStartedSignal();

private:
    bool canBeSent(QByteArray* message);
    void sendMessageImpl(Process* targetProcess,
                         MessageContainer* messageContainer);
    void sendMessageImpl(Process* targetProcess,
                         QByteArray *header,
                         QByteArray *message = nullptr,
                         quint32 messageCounterInit = 1);
    void sendToControlServer(QByteArray *header,
                             QByteArray *message = nullptr,
                             quint32 messageCounterInit = 1);
    void sendToServiceSender(const QString serviceName,
                             QByteArray* header,
                             QByteArray *message = nullptr);
    void sendToServiceReceivers(const QString serviceName,
                                QByteArray* header,
                                QByteArray *message = nullptr);
    void sendProcedureMessageImpl(QString procedureName,
                                  QString procedureMessageType,
                                  QString address,
                                  int port,
                                  int callerId,
                                  QByteArray* message = nullptr);

    QString createHeaderKey(const QStringList &headerList);
    QByteArray * createHeader(const QStringList &headerList);

    QThread* serverThread;
    QEventLoop* senderEventLoop;
    Server* server;

    quint32 numberOfOpenSockets;

    quint32 currentThreadIndex;
    QThread* senderThreads[NUMBER_OF_SENDER_THREADS];

    QList<Socket*> sockets;
    QMutex processSocketLock;
};

#endif
