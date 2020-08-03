#include "procedure.h"

Procedure::Procedure(QString procedureNameInit, QObject *parent) :
    QObject(parent)
{
    procedureName = procedureNameInit;
}

void Procedure::addSender(Process* senderProcess)
{
    sender = senderProcess;
}

void Procedure::removeSender()
{
    sender = NULL;
}

Procedure::~Procedure()
{
    sender = NULL;
}
