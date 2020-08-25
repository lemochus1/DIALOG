#include "process.h"

Process::Process(QString processAddressInit,
                 quint16 processPortInit,
                 ProcessType processTypeInit,
                 QString processNameInit,
                 quint64 processPIDInit)
{
    processAddress = processAddressInit;
    processPort = processPortInit;
    processType = processTypeInit;
    processName = processNameInit;
    processPID = processPIDInit;
    processKey = processAddress + SEPARATOR + QString::number(processPort);
    receiverSocket = nullptr;
    senderSocket = nullptr;
    connectedToControlServer = false;
}

bool Process::addServiceAsSender(Service* service)
{
    bool added = false;
    if (!servicesAsSender.contains(service))
    {
        servicesAsSender.append(service);
        added = true;
    }
    return added;
}

void Process::removeServiceAsSender(Service* service)
{
    if (servicesAsSender.size() > 0)
        servicesAsSender.removeAll(service);
}

bool Process::addServiceAsReceiver(Service* service)
{
    bool added = false;
    if (!servicesAsReceiver.contains(service))
    {
        servicesAsReceiver.append(service);
        added = true;
    }
    return added;
}

void Process::removeServiceAsReceiver(Service* service)
{
    if (servicesAsReceiver.size() > 0)
        servicesAsReceiver.removeAll(service);
}

bool Process::addCommand(Command* command)
{
    bool added = false;
    if (!commands.contains(command))
    {
        commands.append(command);
        added = true;
    }
    return added;
}

bool Process::addProcedure(Procedure *procedure)
{
    bool added = false;
    if (!procedures.contains(procedure))
    {
        procedures.append(procedure);
        added = true;
    }
    return added;
}

void Process::removeProcedure(Procedure *procedure)
{
    if (procedures.size() > 0)
        procedures.removeAll(procedure);
}


void Process::removeCommand(Command *command)
{
    if (commands.size() > 0)
        commands.removeAll(command);
}

bool Process::sendMessage(MessageContainer *messageContainer)
{
    bool hasReceiverSocket = false;
    socketLock.lock();
    if (receiverSocket != nullptr && !receiverSocket->disconnectionInitiated)
    {
        hasReceiverSocket = true;
        receiverSocket->setMessage(messageContainer);
    }
    socketLock.unlock();
    return hasReceiverSocket;
}

void Process::setReceiverSocket(Socket *socket)
{
    socketLock.lock();
    receiverSocket = socket;
    socketLock.unlock();
}

Socket* Process::getReceiverSocket()
{
    socketLock.lock();
    Socket* socket = receiverSocket;
    socketLock.unlock();
    return socket;
}

void Process::setSenderSocket(Socket *socket)
{
    socketLock.lock();
    senderSocket = socket;
    socketLock.unlock();
}

Socket* Process::getSenderSocket()
{
    socketLock.lock();
    Socket* socket = senderSocket;
    socketLock.unlock();
    return socket;
}

Process::~Process()
{
    servicesAsSender.clear();
    servicesAsReceiver.clear();
}
