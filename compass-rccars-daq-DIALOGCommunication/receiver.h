#ifndef RECEIVER_H
#define RECEIVER_H

#include <QtNetwork>
#include <QTcpSocket>
#include <iostream>
#include "server.h"
#include "tcpserver.h"

class Server;
class TcpServer;

class Receiver : public QThread
{
    Q_OBJECT

public:
    Receiver(QString receiverAddressInit, quint16 receiverPortInit, Server* serverInit);
    ~Receiver();
    QString getReceiverAddress();
    quint16 getReceiverPort();
    bool anyOpenSockets();
    void closeAllSockets();
    void closeSocket(Socket* socket);
    void addSocket(Socket* socket);
    void removeSocket(Socket* socket);

    bool receiverStarted;
    quint32 numberOfOpenSockets;
    quint32 currentThreadIndex;
    QThread* receiverThreads[NUMBER_OF_RECEIVER_THREADS];
    Server* server;

public Q_SLOTS:
    void run();
    void startThread();
    void stop();
    void socketDisconnectedSlot();

Q_SIGNALS:
    void tcpServerStartedSignal();
    void setSocketSignal();

private:
    TcpServer* tcpServer;

    QString receiverAddress;
    quint16 receiverPort;

    QThread* serverThread;
    QEventLoop* receiverEventLoop;

    QList<Socket*> sockets;
    QMutex processSocketLock;
};

#endif
