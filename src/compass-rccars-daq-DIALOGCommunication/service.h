#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QMutex>
#include "process.h"

class Service
{
public:
    explicit Service(QString serviceNameInit);
    ~Service();
    QString serviceName;
    Process* sender;
    QList<Process*> receivers;
    void addSender(Process* senderProcess);
    bool addReceiver(Process* receiverProcess);
    void removeSender();
    void removeReceiver(Process* receiverProcess);

    QMutex receiversLock;
};

#endif // SERVICE_H
