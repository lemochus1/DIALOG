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
    void sendCommandMessageSlot(QString commandName, QByteArray* message);
    void sendDirectCommandMessageSlot(QString commandName,
                                      QByteArray* message,
                                      QString processName);
    void sendDirectCommandUrlMessageSlot(QString commandName,
                                         QByteArray* message,
                                         QString url,
                                         int port);

    void callProcedureMessageSlot(QString procedureName, QByteArray* message);
    void sendProcedureReturnMessageSlot(QString procedureName,
                                        QByteArray* message,
                                        QString url,
                                        int port);

    void senderErrorSlot(QString error);
    void socketDisconnectedSlot();
    void startThread();
    void stop();

Q_SIGNALS:
    void connectToHostSignal();
    void senderErrorSignal(QString error);
    void senderStartedSignal();

private:
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
