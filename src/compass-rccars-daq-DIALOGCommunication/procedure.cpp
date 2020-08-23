#include "procedure.h"

Procedure::Procedure(QString procedureNameInit, QObject *parent) :
    QObject(parent)
{
    procedureName = procedureNameInit;
}

bool Procedure::addSender(Process* senderProcess)
{
    bool added = false;
    if(!senders.contains(senderProcess))
    {
        senders.append(senderProcess);
        added = true;
    }
    return added;
}

void Procedure::removeSender(Process *senderProcess)
{
    if(senders.size() > 0)
        senders.removeAll(senderProcess);
}


Procedure::~Procedure()
{
}
