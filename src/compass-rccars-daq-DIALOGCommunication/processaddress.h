#ifndef PROCESSADDRESS_H
#define PROCESSADDRESS_H

#include <QString>
#include <QHostAddress>
#include <QHostInfo>

struct ProcessAddress
{
    QString hostName;
    quint16 port;
    QHostAddress address;

    ProcessAddress(const QString& hostNameInit = "", quint16 portInit = 0);

private:
    QHostAddress getAddress(const QString &hostName);
};
#endif // PROCESSADDRESS_H
