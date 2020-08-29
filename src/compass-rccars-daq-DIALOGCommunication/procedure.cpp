#include "procedure.h"

Procedure::Procedure(QString procedureNameInit, QObject *parent) :
    QObject(parent),
    lastSenderIndex(0)
{
    procedureName = procedureNameInit;
}

bool Procedure::addSender(Process* senderProcess)
{
    bool added = false;
    if (!senders.contains(senderProcess))
    {
        senders.append(senderProcess);
        added = true;
    }
    return added;
}

void Procedure::removeSender(Process *senderProcess)
{
    if (senders.size() > 0)
        senders.removeAll(senderProcess);
}

Process *Procedure::getNextSender()
{
    if (senders.empty())
    {
        return nullptr;
    }
    lastSenderIndex++;
    if (senders.size() >= lastSenderIndex)
    {
        lastSenderIndex = 0;
    }
    return senders[lastSenderIndex];
}


Procedure::~Procedure()
{
}
