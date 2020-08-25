#include "processaddress.h"

ProcessAddress::ProcessAddress(const QString &hostNameInit, quint16 portInit)
    : hostName(hostNameInit),
      port(portInit)
{
    address = getAddress(hostNameInit);
}

QHostAddress ProcessAddress::getAddress(const QString &hostName)
{
    QHostInfo info = QHostInfo::fromName(hostName);
    if (info.addresses().isEmpty()){
        return QHostAddress();
    }
    return info.addresses().first();
}
