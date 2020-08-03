#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QtNetwork>
#include <QTcpSocket>
#include "receiver.h"
#include "socket.h"

class Receiver;

class TcpServer : public QTcpServer
{
public:
    TcpServer(QString receiverAddressInit, quint16 receiverPortInit, Receiver* receiverInit, QObject *parent = 0);
    ~TcpServer();
    QString getReceiverAddress();//asi nikdy
    quint16 getReceiverPort();//aso nikdy

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    void sessionOpened(quint16 receiverPortInit);

    QString receiverAddress;
    quint16 receiverPort;

    Receiver* receiver;
};

#endif // TCPSERVER_H
