#ifndef SOCKET_H
#define SOCKET_H

#include <QtNetwork>
#include <QTcpSocket>
#include <QDateTime>
#include <iostream>
#include "define.h"
#include "server.h"
#include "threadsafeqlist.h"

class Server;
class Process;

class Socket : public QObject
{
    Q_OBJECT

public:
    Socket(QObject *parent = 0);
    ~Socket();

    void setMessage(MessageContainer* messageContainerInit,
                    QString receiverAddressInit = "",
                    quint16 receiverPortInit = 0);
    void setSocketDescriptor(qintptr socketDescriptorInit);
    void setServer(Server* serverInit);
    void setProcess(Process* processInit);
    void setSender(const QByteArray& sender);
    QByteArray getSender();

    bool disconnectionInitiated;

public Q_SLOTS:
    void socketErrorSlot(QAbstractSocket::SocketError socketError);
    void writeMessageSlot();
    void readMessageSlot();
    void connectToHostSlot();
    void setSocketSlot();
    void removeSocketFromSenderSlot();
    void removeSocketFromReceiverSlot();
    void checkIdleSocketSlot();
    void disconnectSocketSlot();

Q_SIGNALS:
    void newMessageSignal();
    void messageReceivedSignal(QString senderAddress,
                               quint16 senderPort,
                               QByteArray* header,
                               QByteArray* message);
    void serverMessageReceivedSignal(QString senderAddress,
                                     quint16 senderPort,
                                     QByteArray* header,
                                     QByteArray* message = nullptr);
    void socketErrorSignal(QString error);
    void disconnectSocketSignal();
    void deleteSignal();

private:
    bool senderKnown;

    quint64 lastActionTimeStamp;
    QTimer* checkIdleSocketTimer;
    QTimer* senderTimer;

    QString senderAddress;
    quint16 senderPort;

    QString receiverAddress;
    quint16 receiverPort;

    QTcpSocket* socket;

    qintptr socketDescriptor;
    quint32 messageSize;

    ThreadSafeQList* messages;
    Server* server;
    Process* process;
};

#endif // SOCKET_H
