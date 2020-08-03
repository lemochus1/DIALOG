#include "command.h"

Command::Command(QString commandNameInit, QObject *parent) :
    QObject(parent)
{
    commandName = commandNameInit;
}

bool Command::addReceiver(Process* receiverProcess)
{
    bool added = false;
    if(!receivers.contains(receiverProcess))
    {
        receivers.append(receiverProcess);
        added = true;
    }
    return added;
}

void Command::removeReceiver(Process *receiverProcess)
{
    if(receivers.size() > 0)
        receivers.removeAll(receiverProcess);
}

Command::~Command()
{
}
