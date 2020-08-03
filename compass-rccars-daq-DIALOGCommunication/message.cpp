#include "message.h"

Message::Message(Process* receiverProcessInit, QByteArray* headerInit, QByteArray* messageInit)
{
    receiverProcess = receiverProcessInit;
    header = headerInit;
    message = messageInit;
}

Message::~Message()
{
}

