#include "service.h"

Service::Service(QString serviceNameInit)
{
    serviceName = serviceNameInit;
    sender = nullptr;
}

void Service::addSender(Process* senderProcess)
{
    sender = senderProcess;
}

bool Service::addReceiver(Process* receiverProcess)
{
    receiversLock.lock();
    bool added = false;
    if (!receivers.contains(receiverProcess))
    {
        receivers.append(receiverProcess);
        added = true;
    }
    receiversLock.unlock();
    return added;
}

void Service::removeSender()
{
    sender = nullptr;
}

void Service::removeReceiver(Process *receiverProcess)
{
    receiversLock.lock();
    if (receivers.size() > 0)
        receivers.removeAll(receiverProcess);
    receiversLock.unlock();
}

Service::~Service()
{
    sender = nullptr;
    receivers.clear();
}
